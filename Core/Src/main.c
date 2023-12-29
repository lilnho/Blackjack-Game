/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "deck.h"
#include "UART.h"

#define DELAY 2500000


/* Keep track of card in deck to draw */
uint8_t deckIndex = 0;

/* Keep track of winner */
/* 0: Nothing | 1: Blackjack | 2: Bust */
uint8_t dealerWin = 0;
uint8_t playerWin = 0;

uint16_t money = 1000;
uint16_t currentBet = 0;

/* Private variables ---------------------------------------------------------*/
RNG_HandleTypeDef hrng;

void SystemClock_Config(void);
static void MX_RNG_Init(void);
void print_table(Hand *dealerHand, Hand *playerHand);
void print_card(Card card);
void place_bet(int option);


int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_RNG_Init();

  UART_init();
  RNG_init();


  //Create initial objects
  Hand *dealerHand;
  Hand *playerHand;
  Card deck[52];

  uint8_t dealerScore;
  uint8_t playerScore;

  uint8_t cashed = 0;

  typedef enum{
	  START_GAME,
	  BETTING,
  	  DEAL_FIRST_CARDS,
	  CALCULATE_HANDS,
	  PLAYER_CHOICE,
	  DEALER_FLIPS,
	  DEALER_TURN,
	  ROUND_DONE,
	  CASH_OUT,
	  RESET_HANDS,
	  BUY_BACK_IN,
	  SHUFFLE_DECK
  }state_var_type;

  state_var_type state = START_GAME;

  uint8_t dealerDone = 0;
  char string[7];
  string[0] = '\0';

  //initialize Hand objects
  dealerHand = hand_init();
  playerHand = hand_init();


  while (1)
  {
	  switch(state)
	  {
	  	  case START_GAME:
	  		  UART_print("\033[H");
	  		  UART_print("\033[2J");
	  		  UART_print("\033[15;15H");
	  		  UART_print("Press Y to GAMBLE");

	  		  //stay in this state until player clicks 'y'
	  		  char start = read_input();
	  		  if (start == 'y' || start == 'Y')
	  		  {
	  			  //start the game
	  			  state = BETTING;
	  		  }

	  		  break;

	  	  case BETTING:
	  		  //check if total money > bet
	  		  if (money >= 500)
	  		  {
	  			  place_bet(3);
	  			  state = SHUFFLE_DECK;
	  		  }
	  		  else if (money >= 250)
	  		  {
	  			  place_bet(2);
	  			  state = SHUFFLE_DECK;
	  		  }
	  		  else if (money >= 100)
	  		  {
	  			  place_bet(1);
	  			  state = SHUFFLE_DECK;
	  		  }
	  		  else
	  		  {
	  			  place_bet(0);
	  			  state = BUY_BACK_IN;
	  		  }
	  		  break;

	  	  case BUY_BACK_IN:
	  		  UART_print("\033[H");
	  		  UART_print("\033[2J");
	  		  UART_print("\033[15;15H");
	  		  UART_print("Buy Back In? [Y]");

	  		  //stay in this state until player clicks 'y'
	  		  char buyIn = read_input();
	  		  if (buyIn == 'y' || buyIn == 'Y')
	  		  {
	  			  //start the game
	  			  money = 1000;
	  			  state = BETTING;
	  		  }

	  		  break;

	  	  case SHUFFLE_DECK:
	  		  //initialize and shuffle deck
	  		  deck_init(deck);
	  		  shuffleDeck(deck, 52);

	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}

	  		  state = DEAL_FIRST_CARDS;
	  		  break;

	  	  case DEAL_FIRST_CARDS:
	  		  //deal player's first card face up
	  		  addCard(playerHand, deck[deckIndex]);
	  		  deckIndex++;
	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}

	  		  //deal dealer's first card face down
	  		  addCard(dealerHand, deck[deckIndex]);
	  		  deckIndex++;
	  		  dealerHand->hand[0].visible = 0;
	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}

	  		  //deal players second card faceup
	  		  addCard(playerHand, deck[deckIndex]);
	  		  deckIndex++;
	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}

	  		  //play dealer's second card face up
	  		  addCard(dealerHand, deck[deckIndex]);
	  		  deckIndex++;
	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}

	  		  state = CALCULATE_HANDS;
	  		  break;

	  	  case CALCULATE_HANDS:
	  		  //check if the dealer/player has 21
	  		  dealerScore = calculateHandValue(dealerHand);
	  		  playerScore = calculateHandValue(playerHand);

	  		  //set dealer wins flag
	  		  if (dealerScore == 21)
	  		  {
	  			  dealerWin = 1;
	  			  state = ROUND_DONE;
	  		  }

	  		  //set player wins flag
	  		  if (playerScore == 21)
	  		  {
	  			  playerWin = 1;
	  			  state = ROUND_DONE;
	  		  }

	  		  //check if dealer busts
	  		  if (dealerScore > 21)
	  		  {
	  			  dealerWin = 2;
	  			  state = ROUND_DONE;
	  		  }

	  		  //check if player busts
	  		  if (playerScore > 21)
	  		  {
	  			  playerWin = 2;
	  			  state = ROUND_DONE;
	  		  }

	  		  //if no one wins yet then move to the player's choice
	  		  if ((dealerWin == 0) && (playerWin == 0) && (dealerDone == 0))
	  		  {
	  			  state = PLAYER_CHOICE;
	  		  }

	  		if ((dealerWin == 0) && (playerWin == 0) && (dealerDone == 1))
	  		{
	  			state = ROUND_DONE;
	  		}
	  		  break;

	  	  case PLAYER_CHOICE:
	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}
	  		  UART_print("\033[5;30H");
	  		  UART_print("\033[0K");		//clear line from cursor to the right
	  		  UART_print("Hit[H] or Stand[S]?");


	  		  char choice = read_input();
	  		  //for (int i = 0; i < DELAY; i++){}

	  		  if (choice == 'h' || choice == 'H')
	  		  {
	  			  //deal player another card
	  			  addCard(playerHand, deck[deckIndex]);
	  			  deckIndex++;
	  			  state = CALCULATE_HANDS;
	  		  }
	  		  else if (choice == 's' || choice == 'S')
	  		  {
	  			  state = DEALER_FLIPS;
	  		  }
	  		  break;

	  	  case DEALER_FLIPS:
	  		  //flip dealers face down card face up
	  		  dealerHand->hand[0].visible = 1;
	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}
	  		  //check if dealer hand is > 16
	  		  dealerScore = calculateHandValue(dealerHand);
	  		  if (dealerScore > 16)
	  		  {
	  			  //calculate to see who wins
	  			  dealerDone = 1;
	  			  state = CALCULATE_HANDS;
	  		  }
	  		  else
	  		  {
	  			  state = DEALER_TURN;
	  		  }
	  		  break;

	  	  case DEALER_TURN:
	  		  //add cards to dealer until dealers score is greater than 16
	  		  dealerScore = calculateHandValue(dealerHand);
	  		  print_table(dealerHand, playerHand);
	  		  for (int i = 0; i < DELAY; i++){}

	  		  if (dealerScore < 17)
	  		  {
	  			  //deal more cards to dealer
	  			  addCard(dealerHand, deck[deckIndex]);
	  			  deckIndex++;
	  			  print_table(dealerHand, playerHand);
		  		  for (int i = 0; i < DELAY; i++){}
	  		  }
	  		  else
	  		  {
	  			  dealerDone = 1;
	  			  state = CALCULATE_HANDS;
	  		  }
	  		  break;

	  	  case ROUND_DONE:
	  		  dealerHand->hand[0].visible = 1;
	  		  print_table(dealerHand, playerHand);
	  		  //for (int i = 0; i < DELAY / 10; i++){}

	  		  //print dealers score at end
	  		  UART_print("\033[1;10H");
	  		  string[0] = '\0';
	  		  toString(calculateHandValue(dealerHand), string, 7);
	  		  UART_print(string);

	  		  for (int i = 0; i < DELAY; i++){}

	  		  //dealer and player get blackjack
	  		  if ((dealerWin == 1) && (playerWin == 1))
	  		  {
	  			  //tied
	  			  UART_print("\033[5;30H");
	  			  UART_print("\033[0K");		//clear line from cursor to the right
	  			  UART_print("Push: Player and Dealer Tied");
	  			  money += currentBet;
	  		  }
	  		  //only dealer gets blackjack
	  		  else if (dealerWin == 1)
	  		  {
	  			  //dealer wins
	  			  UART_print("\033[5;30H");
	  			  UART_print("\033[0K");		//clear line from cursor to the right
	  			  UART_print("The House Always Wins");
	  		  }
	  		  else if (playerWin == 1)
	  		  {
	  			  //player wins
	  			  UART_print("\033[5;30H");
	  			  UART_print("\033[0K");		//clear line from cursor to the right
	  			  UART_print("$$ Nice Hand $$");
	  			  money += (currentBet * 2);
	  		  }
	  		  //dealer busts
	  		  else if (dealerWin == 2)
	  		  {
	  			  //player wins
	  			  UART_print("\033[5;30H");
	  			  UART_print("\033[0K");		//clear line from cursor to the right
	  			  UART_print("$$ Nice Hand $$");
	  			  money += (currentBet * 2);
	  		  }
	  		  //player busts
	  		  else if (playerWin == 2)
	  		  {
	  			  //dealer wins
	  			  UART_print("\033[5;30H");
	  			  UART_print("\033[0K");		//clear line from cursor to the right
	  			  UART_print("The House Always Wins");
	  		  }
	  		  //check scores if neither busted or got blackjack
	  		  else if ((dealerWin == 0) && (playerWin == 0))
	  		  {
	  			  if (dealerScore > playerScore)
	  			  {
	  				  //dealer wins
		  			  UART_print("\033[5;30H");
		  			  UART_print("\033[0K");		//clear line from cursor to the right
		  			  UART_print("The House Always Wins");
	  			  }
	  			  else if (dealerScore < playerScore)
	  			  {
	  				  //player wins
		  			  UART_print("\033[5;30H");
		  			  UART_print("\033[0K");		//clear line from cursor to the right
		  			  UART_print("$$ Nice Hand $$");
		  			  money += (currentBet * 2);
	  			  }
	  			  else
	  			  {
	  				  //tie game
		  			  UART_print("\033[5;30H");
		  			  UART_print("\033[0K");		//clear line from cursor to the right
		  			  UART_print("Push: Player and Dealer Tied");
		  			  money += currentBet;
	  			  }
	  		  }

	  		  //update player's money
	  		  UART_print("\033[1;50H");
  			  UART_print("\033[0K");		//clear line from cursor to the right
	  		  UART_print("$");
	  		  string[0] = '\0';
	  		  toString(money, string, 7);
	  		  UART_print(string);
	  		  string[0] = '\0';

	  		  //delay to see ending result better
	  		  for (int i = 0; i < DELAY * 3; i++){}


	  		  state = CASH_OUT;
	  		  break;

	  	  case CASH_OUT:
	  		  UART_print("\033[H");
	  		  UART_print("\033[2J");
	  		  UART_print("\033[15;15H");
	  		  UART_print("CASH OUT? [Y] [N]");
	  		  char cash = read_input();

	  		  if (cash == 'y' || cash == 'Y')
	  		  {
		  		  UART_print("\033[H");
		  		  UART_print("\033[2J");
	  			  for (int i = 0; i < 30; i++)
	  			  {
	  				  for (int j = 0; j < 30; j++)
	  				  {
	  					  UART_print("$");
	  					  for (int i = 0; i < DELAY / 100; i++){}
	  				  }
	  				UART_print("\033[1B");		//move down 1
	  				UART_print("\033[30D");		//move left 30 spaces
	  			  }
	  			  cashed = 1;
	  			  state = RESET_HANDS;
	  		  }
	  		  else if (cash == 'n' || cash == 'N')
	  		  {
	  			  state = RESET_HANDS;
	  		  }
	  		  break;

	  	  case RESET_HANDS:
	  		  //reset globals and hands
	  		  dealerHand->numCards = 0;
	  		  playerHand->numCards = 0;

	  		  dealerScore = 0;
	  		  playerScore = 0;

	  		  dealerWin = 0;
	  		  playerWin = 0;

	  		  dealerDone = 0;

	  		  deckIndex = 0;

	  		  if (cashed == 1)
	  		  {
	  			  cashed = 0;
	  			  state = START_GAME;
	  		  }
	  		  else
	  		  {
	  			  state = BETTING;
	  		  }
	  		  break;

	  	  default:
	  		  break;
	  }


  }

}

void place_bet(int option)
{
	if (option == 3)
	{
		  UART_print("\033[H");
		  UART_print("\033[2J");
		  UART_print("\033[15;10H");
		  UART_print("Place bet: 100[1] 250[2] 500[3]");
		  char bet = read_input();
		  //subtract bet from current money
		  if (bet == '1')
		  {
			  currentBet = 100;
			  money -= currentBet;
		  }
		  if (bet == '2')
		  {
			  currentBet = 250;
			  money -= currentBet;
		  }
		  if (bet == '3')
		  {
			  currentBet = 500;
			  money -= currentBet;
		  }
	}
	else if (option == 2)
	{
		  UART_print("\033[H");
		  UART_print("\033[2J");
		  UART_print("\033[15;10H");
		  UART_print("Place bet: 100[1] 250[2]");
		  char bet = read_input();
		  //subtract bet from current money
		  if (bet == '1')
		  {
			  currentBet = 100;
			  money -= currentBet;
		  }
		  if (bet == '2')
		  {
			  currentBet = 250;
			  money -= currentBet;
		  }
	}
	else if (option == 1)
	{
		  UART_print("\033[H");
		  UART_print("\033[2J");
		  UART_print("\033[15;10H");
		  UART_print("Place bet: 100[1]");
		  char bet = read_input();
		  //subtract bet from current money
		  if (bet == '1')
		  {
			  currentBet = 100;
			  money -= currentBet;
		  }
	}
	//if gets here, means the player does not have enough for minimum bet
	else
	{
		  UART_print("\033[H");
		  UART_print("\033[2J");
		  UART_print("\033[15;10H");
		  UART_print("Sorry you do not have enough money to bet");
  		  for (int i = 0; i < DELAY * 3; i++){}
	}

}

void print_table(Hand *dealerHand, Hand *playerHand)
{
	/* UART print codes */
	char *clearScreen = "\033[2J";
	//char *resetCurs = "\033[H";
	char string[7];
	string[0] = '\0';


	UART_print(clearScreen);

	//print player's money
	UART_print("\033[1;50H");
	UART_print("$");
	toString(money, string, 7);
	UART_print(string);
	string[0] = '\0';

	UART_print("\033[H");
	//UART_print(resetCurs);

	//print dealer's cards
	UART_print("Dealer");
	UART_print("\033[1B");		//move down 1
	UART_print("\033[6D");		//move left 6 spaces

	for (int i = 0; i < dealerHand->numCards; i++)
	{
		//print card then move cursor location
		print_card(dealerHand->hand[i]);
	}

	//print player's cards
	UART_print("\033[1;20H");
	UART_print("Player");
	UART_print("\033[1B");		//move down 1
	UART_print("\033[6D");		//move left 6 spaces

	for (int i = 0; i < playerHand->numCards; i++)
	{

		//print card then move cursor location
		print_card(playerHand->hand[i]);
	}

	UART_print("\033[1;30H");
	toString(calculateHandValue(playerHand), string, 7);
	UART_print(string);


}

void print_card(Card card)
{
	char string[7];
	string[0] = '\0';

	UART_print(" ----- ");
	UART_print("\033[1B");		//move down 1
	UART_print("\033[7D");		//move left 7 spaces

	if (card.visible == 0)
	{
		UART_print("|#####|");
		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|#####|");
		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|#####|");

	}

	//check if is a face card
	else if (strcmp(card.face, "X"))
	{
		UART_print("|");
		UART_print(card.face);
		UART_print("    |");

		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|     |");

		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|    ");
		UART_print(card.face);
		UART_print("|");

	}
	//check if 2 digits
	else if (card.value > 9)
	{
		toString(card.value, string, 7);
		UART_print("|");
		UART_print(string);
		UART_print("   |");

		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|     |");

		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|   ");
		UART_print(string);
		UART_print("|");
	}
	//single digit
	else
	{
		toString(card.value, string, 7);
		UART_print("|");
		UART_print(string);
		UART_print("    |");

		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|     |");

		UART_print("\033[1B");		//move down 1
		UART_print("\033[7D");		//move left 7 spaces
		UART_print("|    ");
		UART_print(string);
		UART_print("|");
	}

	//print bottom of the card
	UART_print("\033[1B");		//move down 1
	UART_print("\033[7D");		//move left 7 spaces
	UART_print(" ----- ");

	UART_print("\033[2B");		//move down 1
	UART_print("\033[7D");		//move left 7 spaces

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
