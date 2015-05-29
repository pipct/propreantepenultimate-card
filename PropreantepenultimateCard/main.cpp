#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <utility>
#include <random>

using std::move;

struct Card {
    int suit; // D, H, S, C (1-4)
    int rank;
    Card(decltype(suit) suit_, decltype(rank) rank_) : suit{suit_}, rank{rank_} {}
    Card(std::mt19937 &mt) {
        decltype(suit) suit_ = std::uniform_int_distribution<>{1, 4}(mt);
        decltype(rank) rank_ = std::uniform_int_distribution<>{1, 13}(mt);
        suit = suit_;
        rank = rank_;
    }
    std::string desc() const {
        std::string res = std::to_string(rank);
        if (rank ==  1) res = "A";
        if (rank == 11) res = "J";
        if (rank == 12) res = "Q";
        if (rank == 13) res = "K";
        switch (suit) {
            case 0: res += u8"♦"; break;
            case 1: res += u8"♥"; break;
            case 2: res += u8"♠"; break;
            case 3: res += u8"♣"; break;
        }
        return res;
    }
    bool operator==(const Card &other) const {
        return suit == other.suit && rank == other.rank;
    }
    bool operator<(const Card &other) const {
        return rank < other.rank || (rank == other.rank && suit < other.suit);
    }
};

class CardFactory {
    std::vector<Card> _remainingCards{};
    int drawnCards = 0;
    std::mt19937 &_mt;
public:
    CardFactory(std::mt19937 &mt) : _mt{mt} {}
    Card getNext() {
        if (_remainingCards.size() == drawnCards) {
            for (auto suit = 0; suit <= 3; ++suit) {
                for (auto rank = 1; rank <= 13; ++rank) {
                    _remainingCards.emplace_back(suit, rank);
                }
            }
            std::shuffle(_remainingCards.end() - 52, _remainingCards.end(), _mt);
        }
        return _remainingCards[drawnCards++];
    }
};


struct CardOption {
    Card card;
    bool is_special;
    uint8_t secondary_option; // 0:   default
    // 1:   use 3/7 jump or skip a turn with 10
    // 0-3: selected suit from an ace
    
    CardOption(Card card_) : CardOption{card_, false, 0} {}
    CardOption(Card card_,
               bool is_special_,
               uint8_t secondary_option_) : card{card_}, is_special{is_special_}, secondary_option{secondary_option_} {}
    
    
    static std::vector<CardOption> potentialCardOptions(Card c) {
        std::vector<CardOption> result = { { c } }; // every card can be played normally
        switch (c.rank) {
            case 1:
                result.emplace_back(c, true, 2);
                result.emplace_back(c, true, 3);
            case 3:
            case 7:
            case 10:
                result.emplace_back(c, true, 1);
            case 2:
            case 5:
            case 11:
                result.emplace_back(c, true, 0);
        }
        return result;
    }
    
    static std::vector<CardOption> potentialCardOptions(std::vector<Card> cards) {
        std::vector<CardOption> options;
        for (Card c : cards)
            for (CardOption co : potentialCardOptions(c))
                options.push_back(co);
        std::sort(options.begin(), options.end());
        std::unique(options.begin(), options.end());
        return options;
    }
    
    std::string desc() const {
        switch (card.rank) {
            case 4:
            case 6:
            case 8:
            case 9:
            case 12:
            case 13:
                return card.desc();
            case 2:
            case 5:
                if (is_special)
                    return card.desc() + " (attack)";
                else
                    return card.desc() + " (non-special)";
            case 11:
                if (is_special)
                    return card.desc() + " (change direction)";
                else
                    return card.desc() + " (non-special)";
            case 3:
            case 7:
                if (!is_special)
                    return card.desc() + " (non-special)";
                else {
                    if (!secondary_option)
                        return card.desc() + " (block)";
                    else
                        return card.desc() + " (block everything)";
                }
            case 10:
                if (!is_special)
                    return card.desc() + " (non-special)";
                else {
                    if (!secondary_option)
                        return card.desc() + " (extra turn)";
                    else
                        return card.desc() + " (skip next player's turn)";
                }
            case 1:
                if (!is_special)
                    return card.desc() + " (non-special)";
                else {
                    switch (secondary_option) {
                        case 0: return card.desc() + " (change suit to " + u8"♦" + ")";
                        case 1: return card.desc() + " (change suit to " + u8"♥" + ")";
                        case 2: return card.desc() + " (change suit to " + u8"♠" + ")";
                        case 3: return card.desc() + " (change suit to " + u8"♣" + ")";
                    }
                }
            default:
                return std::string{"Error, unknown rank "} + std::to_string(card.rank);
        }
    }
    
    bool operator==(const CardOption &other) const {
        return card == other.card && is_special == other.is_special && secondary_option == other.secondary_option;
    }
    
    bool operator<(const CardOption &other) const {
        if (card < other.card)
            return true;
        if (card == other.card) {
            if (is_special < other.is_special)
                return true;
            if (is_special == other.is_special)
                return secondary_option < other.secondary_option;
        }
        return false;
    }
};

int isGameOver(std::vector<std::vector<Card>> &players) {
    int i = 0;
    for (auto &hand : players) {
        if (hand.size() == 0)
            return i;
        ++i;
    }
    return -1;
}

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
void printOptions(const std::vector<CardOption> &options) {
    for (int i = 0; i < options.size(); ++i) {
        const auto &cardOption = options[i];
        std::cout << "  " << (i + 1) << ": " << cardOption.desc() << "\n";
    }
}
int chooseCardOption(const std::vector<CardOption> &options, std::vector<Card> &playedCards) {
    if (options.size() == 0)
        return -1;
    printPreviousCards(playedCards);
    std::cout << "  Choose a card:\n";
    printOptions(options);
    std::cout << "  > ";
    int i = 0;
    scanf("%i", &i);
    return (i - 1);
}

int main(int argc, const char * argv[]) {
    std::mt19937 mt{/*std::random_device{}()*/};
    CardFactory cardFactory{mt};
    int av = 0, prevAv = 0, mv = 0, currentPlayer = 0;
    bool cw = true;
    std::vector<std::vector<Card>> players;
    std::vector<Card> playedCards;
    auto drawCard = [&cardFactory] { return cardFactory.getNext(); };
    auto playCard = [&players, &playedCards, &currentPlayer] (Card card) {
        auto &cardsInHand = players[currentPlayer];
        int idx = 0;
        for (auto c : cardsInHand) {
            if (c == card)
                break;
            ++idx;
        }
        cardsInHand.erase(cardsInHand.begin() + idx);
        playedCards.push_back(card);
    };
    auto pickUpCards = [&drawCard, &players, &currentPlayer] (int num = 1) {
        for (int i = 0; i < num; ++i) {
            auto card = drawCard();
            players[currentPlayer].push_back(card);
            std::cout << "Picked up " << card.desc() << "\n";
        }
    };
    auto topCard = [&playedCards] { return playedCards.back(); };
    auto previousTopCard = [&playedCards] { return playedCards.size() >= 2 ? &playedCards[playedCards.size() - 2] : nullptr; };
    auto isbCardCount = -1ul;
    auto areSquareBracketsIgnored = [&playedCards, &isbCardCount] { return playedCards.size() == isbCardCount; };
    auto ignoreSquareBrackets = [&playedCards, &isbCardCount] { isbCardCount = playedCards.size(); };
    
    // deal cards to players
    for (int p = 0; p < 2; ++p) {
        std::vector<Card> hand;
        for (int c = 0; c < 7; ++c) {
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
    
    // start the game
    while (isGameOver(players) == -1) {
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
        auto cardOptions = std::vector<CardOption>{};
        {
            auto potentialCardOptions = CardOption::potentialCardOptions(players[currentPlayer]);
            for (auto option : potentialCardOptions) {
                
                if (!firstCardThisTurn) {
                    // test if it can form a valid chain
                    auto a = option.card;
                    auto b = topCard();
                    auto c = previousTopCard(); // ptr (unlike a and b)
                    
                    if (option.card.rank == topCard().rank) { // A
                        ignoreSquareBrackets();
                    } else if (((option.card.rank == 3 && topCard().rank == 7) // C
                                || (option.card.rank == 7 && topCard().rank == 3))
                               && prevAv > 0) {
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
                    else if (option.card.rank == topCard().rank + 1 // B (last so that
                                                                    // ignoreSquareBrackets()
                                                                    // is called if possible)
                             || option.card.rank == topCard().rank - 1
                             || (option.card.rank == 1 && topCard().rank == 13)
                             || (option.card.rank == 1 && topCard().rank == 13)) {
                        // no ignoreSquareBrackets because suit matches remain
                        // required for +-1 chains
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
        int selectedOptionIdx = chooseCardOption(cardOptions, playedCards);
        if (selectedOptionIdx == -1) {
            if (firstCardThisTurn) {
                pickUpCards(std::max(1, av));
                av = 0;
            }
            continue;
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
                case 5: av += 3;
                case 2: av += 2; break;
                case 3:
                case 7:
                    av -= card.rank;
                    if (av < 0) av = 0;
                    if (selectedOption.secondary_option == 1)
                        // 3 to 7 bridge
                        av = 0;
                    if (players[currentPlayer].size() == 1)
                        // last card
                        mv = 0;
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
    std::cout << "Game won by player " << (isGameOver(players) + 1) << ".";
    return 0;
}
