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
    std::string desc() {
        std::string res = std::to_string(rank);
        if (rank ==  1) res = "A";
        if (rank == 11) res = "J";
        if (rank == 12) res = "Q";
        if (rank == 13) res = "K";
        switch (suit) {
            case 1: res += u8"♦"; break;
            case 2: res += u8"♥"; break;
            case 3: res += u8"♠"; break;
            case 4: res += u8"♣"; break;
        }
        return res;
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
            for (auto suit = 1; suit <= 4; ++suit) {
                for (auto rank = 1; rank <= 13; ++rank) {
                    _remainingCards.emplace_back(suit, rank);
                }
            }
            std::shuffle(_remainingCards.end() - 52, _remainingCards.end(), _mt);
        }
        return _remainingCards[drawnCards++];
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

void printPreviousCards(std::vector<Card> &playedCards) {
    std::cout << "  Cards currently on the pile (bottom to top):\n";
    for (int i = std::max(0, static_cast<int>(playedCards.size()) - 3); i < playedCards.size(); ++i) {
        Card &card = playedCards[i];
        std::cout << "  " << (i + 1) << ": " << card.desc() << "\n";
    }
}
void printHand(std::vector<Card> &hand) {
    for (int i = 0; i < hand.size(); ++i) {
        Card &card = hand[i];
        std::cout << "  " << (i + 1) << ": " << card.desc() << "\n";
    }
}
int chooseCard(std::vector<Card> &hand, std::vector<Card> &playedCards) {
    printPreviousCards(playedCards);
    std::cout << "  Choose a card:\n";
    printHand(hand);
    std::cout << "  > ";
    int i = 0;
    scanf("%i", &i);
    return (i - 1);
}

int main(int argc, const char * argv[]) {
    std::mt19937 mt{std::random_device{}()};
    CardFactory cardFactory{mt};
    int av = 0, mv = 0, currentPlayer = 0;
    bool cw = true;
    std::vector<std::vector<Card>> players;
    std::vector<Card> playedCards;
    auto drawCard = [&cardFactory] { return cardFactory.getNext(); };
    auto playCard = [&players, &playedCards, &currentPlayer] (int cardIdx) {
        auto card = players[currentPlayer][cardIdx];
        players[currentPlayer].erase(players[currentPlayer].begin() + cardIdx);
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
    //auto previousTopCard = [&playedCards] { return playedCards.size() >= 2 ? &playedCards[playedCards.size() - 2] : nullptr; };
    auto isbCardCount = -1ul;
    //auto areSquareBracketsIgnored = [&playedCards, &isbCardCount] { return playedCards.size() == isbCardCount; };
    auto ignoreSquareBrackets = [&playedCards, &isbCardCount] { isbCardCount = playedCards.size(); };
    
    // deal cards to players
    for (int p = 0; p < 2; ++p) {
        std::vector<Card> hand;
        for (int c = 0; c < 7; ++c) {
            hand.push_back(drawCard());
        }
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
        
        // step 2
        if (mv < 1) ++mv;
        if (mv > 1) mv = 1;
        
        // step 3
    chooseFirstCard:
        int selectedFirstCard = chooseCard(players[currentPlayer], playedCards);
        if (selectedFirstCard == -1) {
            pickUpCards(std::max(1, av));
            av = 0;
            continue;
        }
        auto card = players[currentPlayer][selectedFirstCard];
        if (card.rank == topCard().rank || card.suit == topCard().suit) {
            playCard(selectedFirstCard);
        } else {
            std::cout << "  Invalid move, please choose a different card.\n";
            goto chooseFirstCard;
        }
    }
    std::cout << "Game won by player " << (isGameOver(players) + 1) << ".";
    return 0;
}
