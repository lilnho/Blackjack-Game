/*
 * deck.c
 *
 *  Created on: Jun 3, 2023
 *      Author: natha
 */

#include "deck.h"
#include <stdlib.h>


//Configure the built in RNG
void RNG_init(void)
{
	//enable RNG clock
	RCC->AHB2ENR |= (RCC_AHB2ENR_RNGEN);

	//reset RNG
	RCC->AHB2RSTR |= RCC_AHB2RSTR_RNGRST;		//resets RNG
	RCC->AHB2RSTR &= ~(RCC_AHB2RSTR_RNGRST);


	//enable RNGEN for generation
	RNG->CR |= RNG_CR_RNGEN;

	//configure
	RNG->CR |= (1 << 5);		//CED
	RNG->CR &= ~(RNG_CR_IE);	//disable interrupt enable

}

//creates the 52 card deck (not shuffled)
void deck_init(Card deck[])
{
	uint8_t index = 0;
	uint8_t suitsIndex, number, faceCard;
	char suits[4][9] = {"Spades", "Clubs", "Diamonds", "Hearts"};
	char faces[3][2] = {"J", "Q", "K"};

	//put in each suit
	for (suitsIndex = 0; suitsIndex < 4; suitsIndex++)
	{
		//put in each number card
		for (number = 2; number < 11; number++)
		{
			deck[index].value = number;
			deck[index].visible = 1;
			strcpy(deck[index].suit, suits[suitsIndex]);
			strcpy(deck[index].face, "X");
			index++;
		}

		//put in face cards
		for (faceCard = 0; faceCard < 3; faceCard++)
		{
			deck[index].value = 10;
			deck[index].visible = 1;
			strcpy(deck[index].suit, suits[suitsIndex]);
			strcpy(deck[index].face, faces[faceCard]);
			index++;
		}

		//put in Ace
		deck[index].value = 1;
		deck[index].visible = 1;
		strcpy(deck[index].suit, suits[suitsIndex]);
		strcpy(deck[index].face, "A");
		index++;
	}

}

Hand* hand_init(void)
{
	Hand* hand = malloc(sizeof(Hand));
	hand->numCards = 0;
	return hand;
}

void addCard(Hand* hand, Card card)
{
	hand->hand[hand->numCards] = card;
	hand->numCards++;
}

//calculate the total value of target hand
uint8_t calculateHandValue(Hand* hand)
{
    uint8_t totalValue = 0;
    uint8_t numAces = 0;

    for (uint8_t i = 0; i < hand->numCards; i++)
    {
        totalValue += hand->hand[i].value;

        //check if card is an Ace
        if (hand->hand[i].value == 1)
        {
            numAces++;
        }

    }

    //change aces value depending on total
    while (numAces > 0 && totalValue + 10 <= 21)
    {
        totalValue += 10;
        numAces--;
    }

    return totalValue;
}

//get rng value from RNG data register
uint32_t getRNG(void)
{
	uint32_t data = 0;

	//wait until rng is ready
	while (!(RNG->SR & RNG_SR_DRDY)){}

	data = RNG->DR;
	return data;
}

//shuffle array using the rng value
void shuffleDeck(Card deck[], int size)
{
	for (int i = 0; i < size; i++)
	{
		int swap = getRNG() % (size - 1);
		Card temp = deck[i];
		deck[i] = deck[swap];
		deck[swap] = temp;
	}
}




