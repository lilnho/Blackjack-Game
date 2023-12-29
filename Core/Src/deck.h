/*
 * deck.h
 *
 *  Created on: Jun 3, 2023
 *      Author: natha
 */

#ifndef SRC_DECK_H_
#define SRC_DECK_H_

#include <stdint.h>
#include <string.h>
#include "stm32l4xx_hal.h"

typedef struct
{
	uint8_t value;
	uint8_t visible;	//determine if facedown or faceup
	char face[2];	//letter value for face cards
	char suit[9];	//spades, diamonds, clubs, hearts

}Card;

typedef struct {
    Card hand[11];	//store the cards in the hand
    int numCards;
} Hand;



void RNG_init(void);
void deck_init(Card deck[]);
Hand* hand_init(void);
void addCard(Hand* hand, Card card);
uint8_t calculateHandValue(Hand* hand);
uint32_t getRNG(void);
void shuffleDeck(Card deck[], int size);




#endif /* SRC_DECK_H_ */
