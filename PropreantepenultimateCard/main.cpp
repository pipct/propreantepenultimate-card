#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <utility>
#include <random>
#include <algorithm>
#include <stdint.h>
#include <sstream>

#ifdef _WIN32
    #define DIAMOND_STR "D"
    #define HEART_STR "H"
    #define SPADE_STR "S"
    #define CLUB_STR "C"
#else
    #define DIAMOND_STR "\xE2\x99\xA6"
    #define HEART_STR "\xE2\x99\xA5"
    #define SPADE_STR "\xE2\x99\xA0"
    #define CLUB_STR "\xE2\x99\xA3"
#endif

struct Card {
    uint8_t suit; // D, H, S, C (1-4)
    uint8_t rank;
    Card(uint8_t suit_, uint8_t rank_) : suit(suit_), rank(rank_) {}
    Card(std::mt19937 &mt) {
        suit = std::uniform_int_distribution<>(1, 4)(mt);
        rank = std::uniform_int_distribution<>(1, 13)(mt);
    }
    Card() : suit(0), rank(0) {}
    std::string desc() const {
		std::stringstream ss;
		ss << static_cast<int>(rank);
		std::string res = ss.str();
        if (rank ==  1) res = "A";
        if (rank == 11) res = "J";
        if (rank == 12) res = "Q";
        if (rank == 13) res = "K";
        switch (suit) {
            case 0:  return res + DIAMOND_STR;
            case 1:  return res + HEART_STR;
            case 2:  return res + SPADE_STR;
            default: return res + CLUB_STR;
        }
    }
    bool operator==(const Card &other) const {
        return suit == other.suit && rank == other.rank;
    }
    bool operator<(const Card &other) const {
        return rank < other.rank || (rank == other.rank && suit < other.suit);
    }
    bool potentially_special() const {
        return rank == 1 || rank == 2 || rank == 3
            || rank == 5 || rank == 7 || rank == 10 || rank == 11;
    }
};

class CardFactory {
    std::vector<Card> _remainingCards;
    std::mt19937 &_mt;
    uint8_t drawnCards;
public:
    CardFactory(std::mt19937 &mt)
      : _mt(mt), _remainingCards(52), drawnCards(52) {}
    Card getNext() {
        if (drawnCards == 52) {
            int i = 0;
            for (uint8_t suit = 0; suit <= 3; ++suit) {
                for (uint8_t rank = 1; rank <= 13; ++rank) {
                    _remainingCards[i] = Card(suit, rank);
                    ++i;
                }
            }
            std::random_shuffle(_remainingCards.begin(), _remainingCards.end());
            drawnCards = 0;
        }
        return _remainingCards[drawnCards++];
    }
};

struct CardOption {
    Card card;
    bool is_special;
    uint8_t secondary_option;
    // secondary_option = 1:
    //   - Block everything with a 3/7 jump
    //   - Skip next player's turn with a 10
    // secondary_option = 0-3:
    //   - Selected Ace suit

    CardOption(Card card_)
      : card(card_), is_special(false), secondary_option(0) {}
    CardOption(Card card_,
               bool is_special_,
               uint8_t secondary_option_)
      : card(card_),
        is_special(is_special_),
        secondary_option(secondary_option_) {}


    static std::vector<CardOption> potentialCardOptions(Card c) {
		CardOption cardOption = c;
        std::vector<CardOption> result; // every card can be played normally
		result.push_back(cardOption);
        switch (c.rank) {
            case 1:
                result.push_back(CardOption(c, true, 2));
                result.push_back(CardOption(c, true, 3));
            case 3:
            case 7:
            case 10:
                result.push_back(CardOption(c, true, 1));
            case 2:
            case 5:
            case 11:
                result.push_back(CardOption(c, true, 0));
        }
        return result;
    }

    static std::vector<CardOption> potentialCardOptions(
      std::vector<Card> cards) {
        std::sort(cards.begin(), cards.end());
        std::unique(cards.begin(), cards.end());
        std::vector<CardOption> options;
		for (int i = 0; i < cards.size(); ++i) {
			Card c = cards[i];
			std::vector<CardOption> potential_options = potentialCardOptions(c);
            for (int j = 0; j < potential_options.size(); ++j)
				options.push_back(potential_options[j]);
		}
        return options;
    }

    std::string desc() const {
        if (is_special) {
            switch (card.rank) {
                case 2:
                case 5:
                    return card.desc() + " (attack)";
                case 11:
                    return card.desc() + " (change direction)";
                case 3:
                case 7:
                    if (!secondary_option)
                        return card.desc() + " (block)";
                    else
                        return card.desc() + " (block everything)";
                case 10:
                    if (!secondary_option)
                        return card.desc() + " (extra turn)";
                    else
                        return card.desc() + " (skip next player's turn)";
                case 1:
                    switch (secondary_option) {
                        case 0: return card.desc() + " (change suit to " + DIAMOND_STR + ")";
                        case 1: return card.desc() + " (change suit to " + HEART_STR + ")";
                        case 2: return card.desc() + " (change suit to " + SPADE_STR + ")";
                        case 3: return card.desc() + " (change suit to " + CLUB_STR + ")";
                    }
            }
        }
        if (card.potentially_special()) {
            return card.desc() + " (non-special)";
        }

        return card.desc();
    }

    bool operator==(const CardOption &other) const {
        return card == other.card && is_special == other.is_special && secondary_option == other.secondary_option;
    }
};

void printPreviousCards(const std::vector<Card> &playedCards) {
    std::cout << "  Cards currently on the pile (bottom to top):\n";
    for (int i = std::max(0, static_cast<int>(playedCards.size()) - 3); i < playedCards.size(); ++i) {
        const auto &card = playedCards[i];
        std::cout << "  " << (i + 1) << ": " << card.desc() << "\n";
    }
}
void printHand(const std::vector<Card> &hand) {
    for (int i = 0; i < hand.size(); ++i) {
        const auto &card = hand[i];
        std::cout << "  " << (i + 1) << ": " << card.desc() << "\n";
    }
}
void printOptions(const std::vector<CardOption> &options, int pickupValue) {
    if (pickupValue == 0)
        std::cout << "  0: End turn\n";
    else
        std::cout << "  0: End turn (pick up " << pickupValue << " cards)\n";
    for (int i = 0; i < options.size(); ++i) {
        const auto &cardOption = options[i];
        std::cout << "  " << (i + 1) << ": " << cardOption.desc() << "\n";
    }
}
int chooseCardOption(const std::vector<CardOption> &options, std::vector<Card> &playedCards, int pickupValue) {
    if (options.size() == 0)
        return -1;
    printPreviousCards(playedCards);
    std::cout << "  Choose an action:\n";
    printOptions(options, pickupValue);
get_input:
    std::cout << "  > ";
    int result = -1;
    std::string input;
    std::cin >> input;
	for (int i = 0; i < input.size(); ++i) {
		char c = input[i];
        if (c < '0' || c > '9')
            break;
        if (result == -1)
            result = 0;
        result = 10 * result + (c - '0');
    }
    if (result < 0 || result > options.size()) {
        std::cout << "Invalid input. Please enter a number between 0 and " << options.size() << ".\n";
        goto get_input;
    }
    return (result - 1);
}

struct GameState {
    std::mt19937 &mt;
    CardFactory cardFactory{mt};
    int av, mv, currentPlayer;
    bool cw;
    int turnCount;
    unsigned long isbCardCount;
    int allowChain3And7TurnCount;
    int preventGameOverTurnCount;
    std::vector<std::vector<Card>> players;
    std::vector<Card> playedCards;

    int isGameOver() {
        if (isGameOverPreventedThisTurn())
            return 0;
        for (int i = 0; i < players.size(); ++i) {
            auto hand = players[i];
            if (hand.size() == 0)
                return i;
            ++i;
        }
        return 0;
    }

    Card drawCard() {
        return cardFactory.getNext();
    }
    void playCard(Card card) {
        auto &cardsInHand = players[currentPlayer];
        int idx = 0;
		for (int i = 0; i < cardsInHand.size(); ++i) {
			Card c = cardsInHand[i];
            if (c == card)
                break;
            ++idx;
        }
        cardsInHand.erase(cardsInHand.begin() + idx);
        playedCards.push_back(card);
    };
    void pickUpCards(int num = 1) {
        for (int i = 0; i < num; ++i) {
            auto card = drawCard();
            players[currentPlayer].push_back(card);
            std::cout << "Picked up " << card.desc() << "\n";
        }
    };
    Card topCard() {
        return playedCards.back();
    };
    Card *previousTopCard() {
        return playedCards.size() >= 2 ? &playedCards[playedCards.size() - 2] : nullptr;
    };
    bool areSquareBracketsIgnored() {
        return playedCards.size() == isbCardCount;
    };
    void ignoreSquareBrackets() {
        isbCardCount = playedCards.size();
    };
    bool are3And7ChainsAllowed() {
        return turnCount == allowChain3And7TurnCount;
    };
    void allow3And7Chain() {
        allowChain3And7TurnCount = turnCount;
    };
    bool isGameOverPreventedThisTurn() {
        return turnCount == preventGameOverTurnCount;
    };
    void preventGameOver() {
        preventGameOverTurnCount = turnCount;
    };

	GameState(std::mt19937 &mt_, int numPlayers, int numCards)
    : mt(mt_),
      av(0),
      cardFactory(mt),
      mv(0),
      currentPlayer(0),
      cw(true),
      isbCardCount(-1),
      allowChain3And7TurnCount(-1),
      preventGameOverTurnCount(-1) {
        // deal cards to players
        for (int p = 0; p < numPlayers; ++p) {
            std::vector<Card> hand;
            for (int c = 0; c < numCards; ++c) {
                hand.push_back(drawCard());
            }
            std::sort(hand.begin(), hand.end());
            players.push_back(hand);
        }

        // initialize
        playedCards.push_back(drawCard());
        switch (topCard().rank) {
            case 2: av = 2; break;
            case 5: av = 5; break;
            case 11: cw = false; // fallthrough
            case 10: mv = 1; break;
            case 1: ignoreSquareBrackets(); break;
        }
        printPreviousCards(playedCards);
    }

    void playTurn() {
        ++turnCount;

        // step 1
        if (cw)
            currentPlayer += std::max(mv, 0);
        else
            currentPlayer -= std::max(mv, 0);
        while (currentPlayer < 0 || currentPlayer >= players.size()) {
            if (currentPlayer < 0)
                currentPlayer += players.size();
            else
                currentPlayer -= players.size();
        }

        std::cout << "\n\nPlayer " << (currentPlayer + 1) << ":\n";
        printHand(players[currentPlayer]);

        // step 2
        if (mv < 1) ++mv;
        if (mv > 1) mv = 1;

        // step 3
        auto firstCardThisTurn = true;
    chooseCard:
        std::vector<CardOption> cardOptions;
        {
            auto potentialCardOptions = CardOption::potentialCardOptions(players[currentPlayer]);
			for (int E = 0; E < potentialCardOptions.size(); ++E) {
				auto option = potentialCardOptions[E];

                if (!firstCardThisTurn) {
                    // test if it can form a valid chain
                    auto a = option.card;
                    auto b = topCard();
                    auto c = previousTopCard(); // ptr (unlike a and b)

                    if (option.card.rank == topCard().rank) { // A
                        ignoreSquareBrackets();
                    } else if (((option.card.rank == 3 && topCard().rank == 7) // C
                                || (option.card.rank == 7 && topCard().rank == 3))
                               && are3And7ChainsAllowed()) {
                        ignoreSquareBrackets();
                    } else if (previousTopCard() != nullptr // D
                               && option.card.rank >= 2
                               && option.card.rank <= 10
                               && topCard().rank >= 2
                               && topCard().rank <= 10
                               && previousTopCard()->rank >= 2
                               && previousTopCard()->rank <= 10
                               && (0
                                   || a.rank + b.rank == c->rank
                                   || a.rank + c->rank == b.rank
                                   || b.rank + c->rank == a.rank

                                   || a.rank - b.rank == c->rank
                                   || a.rank - c->rank == b.rank
                                   || b.rank - c->rank == a.rank
                                   || b.rank - a.rank == c->rank
                                   || c->rank - a.rank == b.rank
                                   || c->rank - b.rank == a.rank

                                   || a.rank * b.rank == c->rank
                                   || a.rank * c->rank == b.rank
                                   || b.rank * c->rank == a.rank

                                   || (a.rank / b.rank == c->rank && a.rank % b.rank == 0)
                                   || (a.rank / c->rank == b.rank && a.rank % c->rank == 0)
                                   || (b.rank / c->rank == a.rank && b.rank % c->rank == 0)
                                   || (b.rank / a.rank == c->rank && b.rank % a.rank == 0)
                                   || (c->rank / a.rank == b.rank && c->rank % a.rank == 0)
                                   || (c->rank / b.rank == a.rank && c->rank % b.rank == 0)))
                        ignoreSquareBrackets();
                    else if (option.card.rank == topCard().rank + 1
                             // B (last so that ignoreSquareBrackets() is called if possible)
                             || option.card.rank == topCard().rank - 1
                             || (option.card.rank == 1 && topCard().rank == 13)
                             || (option.card.rank == 13 && topCard().rank == 1)) {
                        // no ignoreSquareBrackets() because suit matches remain
                        // required for +/- 1 chains
                    } else {
                        continue;
                    }
                }

                // validate options
                if (!option.is_special) {
                    if (av != 0)
                        continue;
                    if (!areSquareBracketsIgnored() && option.card.suit != topCard().suit && option.card.rank != topCard().rank)
                        continue;
                } else {
                    switch (option.card.rank) {
                        case 2:
                        case 5:
                            if (players[currentPlayer].size() == 1) // last card
                                continue;
                            break;
                        case 3:
                        case 7:
                            if (players[currentPlayer].size() == 1 && av == 0) // last card and no attack
                                continue;
                            if (option.secondary_option == 1) {
                                // 3 to 7 bridge
                                if (option.card.rank == 3 && topCard().rank != 7)
                                    continue;
                                if (option.card.rank == 7 && topCard().rank != 3)
                                    continue;
                                if (firstCardThisTurn)
                                    continue; // bridge must be played on one turn
                            }
                            break;
                        case 10:
                            if (!areSquareBracketsIgnored() && option.card.suit != topCard().suit && option.card.rank != topCard().rank)
                                continue;
                            if (players[currentPlayer].size() == 1) // last card
                                continue;
                            // rules apply regardless of option.secondary_option
                            break;
                        case 11:
                            if (players[currentPlayer].size() == 1) // last card
                                continue;
                            break;
                        case 1:
                            if (av > 0)
                                continue;
                            if (players[currentPlayer].size() == 1) // last card
                                continue;
                            break;
                        default:
                            continue;
                    }
                }
                cardOptions.push_back(option);
            }
        }
        // returns -1 if user didn't choose or if there are no options
        int selectedOptionIdx = chooseCardOption(cardOptions, playedCards, firstCardThisTurn ? std::max(1, av) : 0);
        if (selectedOptionIdx == -1) {
            if (firstCardThisTurn) {
                pickUpCards(std::max(1, av));
                av = 0;
            }
            return;
        }
        if (selectedOptionIdx >= cardOptions.size()) {
            std::cout << "  Invalid move, please choose a different option.";
            goto chooseCard;
        }
        auto selectedOption = cardOptions[selectedOptionIdx];
        auto card = selectedOption.card;
        playCard(card);
        if (selectedOption.is_special) {
            switch (card.rank) {
                case 2: av += 2; break;
                case 5: av += 5; break;
                case 3:
                case 7:
                    if (av > 0)
                        allow3And7Chain();
                    av -= card.rank;
                    if (av < 0) av = 0;
                    if (selectedOption.secondary_option == 1)
                        // 3 to 7 bridge
                        av = 0;
                    if (players[currentPlayer].size() == 1) {
                        // last card
                        mv = 0;
                        preventGameOver();
                    }
                    break;
                case 10:
                    if (selectedOption.secondary_option)
                        ++mv;
                    else
                        --mv;
                    break;
                case 11:
                    cw = !cw;
                    break;
                case 1:
                    playedCards.back().suit = selectedOption.secondary_option;
                    break;
            }
        }
        firstCardThisTurn = false;
        goto chooseCard;
    }
};

int main(int argc, const char *argv[]) {
	std::random_device rd;
    std::mt19937 mt(rd());
    GameState game(mt, 3, 7);

    while (!game.isGameOver()) {
        game.playTurn();
    }

    std::cout << "Game won by player " << game.isGameOver() << ".";
#ifdef _WIN32
    std::string s;
    std::cin >> s;
#endif
    return 0;
}
