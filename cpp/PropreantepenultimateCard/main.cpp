#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <utility>
#include <random>
#include <algorithm>
#include <stdint.h>
#include <sstream>
#include <stdlib.h>
#include <numeric>

#include "coroutine.h"

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

#define DIAMOND_SUIT 0
#define HEART_SUIT 1
#define SPADE_SUIT 2
#define CLUB_SUIT 3

struct Card {
    uint8_t suit; // D, H, S, C (0-3)
    uint8_t rank;
    uint8_t realSuit;
    uint8_t realRank;
    Card(uint8_t suit_, uint8_t rank_) : suit(suit_), realSuit(suit_), rank(rank_), realRank(rank_) {}
    Card(std::mt19937 &mt) {
        realSuit = suit = std::uniform_int_distribution<>(1, 4)(mt);
        realRank = suit = std::uniform_int_distribution<>(1, 13)(mt);
    }
    Card() : suit(0), realSuit(0), rank(0), realRank(0) {}
    std::string desc() const {
        if (realRank == 0 || rank == 0)
            return "(null)";
        std::string res;
        {
            std::stringstream ss;
            ss << static_cast<int>(realRank);
            res = ss.str();
        }
        if (realRank ==  1) res = "A";
        if (realRank == 11) res = "J";
        if (realRank == 12) res = "Q";
        if (realRank == 13) res = "K";
        switch (realSuit) {
            case DIAMOND_SUIT: res += DIAMOND_STR; break;
            case HEART_SUIT:   res += HEART_STR;   break;
            case SPADE_SUIT:   res += SPADE_STR;   break;
            default:           res += CLUB_STR;    break;
        }
        if (realSuit == suit && realRank == rank)
            return res;
        res += " (treated as ";
        if (rank ==  1) res += "A";
        else if (rank == 11) res += "J";
        else if (rank == 12) res += "Q";
        else if (rank == 13) res += "K";
        else {
            std::stringstream ss;
            ss << static_cast<int>(rank);
            res += ss.str();
        }
        switch (suit) {
            case DIAMOND_SUIT: res += DIAMOND_STR; break;
            case HEART_SUIT:   res += HEART_STR; break;
            case SPADE_SUIT:   res += SPADE_STR; break;
            default:           res += CLUB_STR; break;
        }
        return res + ")";
    }
    bool operator==(const Card &other) const {
        return realSuit == other.realSuit && realRank == other.realRank;
    }
    bool operator<(const Card &other) const {
        return realRank < other.realRank || (realRank == other.realRank && realSuit < other.realSuit);
    }
    bool potentially_special() const {
        return false;
    }
};

class CardFactory {
public:
    std::vector<Card> _remainingCards;
    std::mt19937 &_mt;
    uint8_t _drawnCards;
    CardFactory(std::mt19937 &mt) : _mt(mt), _remainingCards(52), _drawnCards(52) {}
    Card getNext() {
        if (_drawnCards == 52) {
            int i = 0;
            for (uint8_t suit = 0; suit <= 3; ++suit) {
                for (uint8_t rank = 1; rank <= 13; ++rank) {
                    _remainingCards[i] = Card(suit, rank);
                    ++i;
                }
            }
            std::random_shuffle(_remainingCards.begin(), _remainingCards.end());
            _drawnCards = 0;
        }
        return _remainingCards[_drawnCards++];
    }
};

struct GameInitSettings {
    int numPlayers, numCards;
    std::mt19937 mt;
    CardFactory cardFactory;
    GameInitSettings(int np, int nc, std::mt19937 mt_) : numPlayers(np), numCards(nc), mt(mt_), cardFactory(mt_) {}
};

enum class OptionType : uint8_t {
    Null = 0,
    NoCard, ReplaceHand, Attack, Block, ChangeMV, ChangeDirection, ChangeSuit, NormalCard,
    NormalBlockOnly, BlockEverything,
    HaveAnotherTurn, SkipNextPlayersTurn,
    ChooseSuitDiamonds, ChooseSuitHearts, ChooseSuitSpades, ChooseSuitClubs,
    PlayVertically, PlayHorizontally
};
struct Option { OptionType type; std::string message; Card card; };
bool operator<(const Option &a, const Option &b) {
    if (a.type < b.type)
        return true;
    if (a.type == b.type)
        if (a.card < b.card)
            return true;
    return false;
}
bool operator==(const Option &a, const Option &b) {
    return a.type == b.type && a.card == b.card;
}
using options_t = std::vector<Option>;

options_t propreantepenultimate_card(ccrContParam, int selectedOptionIdx, GameInitSettings initSettings) {
    ccrBeginContext;
    std::vector<std::vector<Card>> players;
    std::vector<Card> playedCards;
    Card topCard() { return playedCards.back(); }
    Card *prevTopCard() { return playedCards.size() >= 2 ? &playedCards[playedCards.size() - 2] : nullptr; }
    Card drawCard(GameInitSettings initSettings) { return initSettings.cardFactory.getNext(); }
    void playCardFromHand(Card card) {
        auto &cardsInHand = players[activePlayer];
        int idx = 0;
        for (int i = 0; i < cardsInHand.size(); ++i) {
            Card c = cardsInHand[i];
            if (c == card)
                break;
            ++idx;
        }
        if (cardsInHand.begin() + idx == cardsInHand.end())
            throw "Error trying to play " + card.desc() + " as it was not in the active player's hand";
        cardsInHand.erase(cardsInHand.begin() + idx);
        playedCards.push_back(card);
    }
    void printInfo() {
        std::cout << "\n\n\n\n\n\n\n--- Player " << (activePlayer + 1) << "'s Turn ---\nPile: ";
        for (auto c : playedCards) {
            std::cout << c.desc() << "\n";
        }
        std::cout << "\nHand: ";
        for (auto c : players[activePlayer])
            std::cout << c.desc() << "\n      ";
        printf("\nav: %d, mv: %d, cw: %d, rm: %d, b: %d, pw: %d\n", av, mv, cw, rm, b, pw);
    }
    int activePlayer, av, mv;
    bool cw, fc, rm, b, pw;
    options_t options;
    Option selectedOption;
    ccrEndContext(ctx);
    
    ccrBegin(ctx);
    ctx->activePlayer = 0, ctx->av = 0, ctx->mv = 0;
    ctx->cw = true, ctx->fc = true, ctx->rm = true, ctx->b = false, ctx->pw = false;
    
    // deal cards to players
    for (int p = 0; p < initSettings.numPlayers; ++p) {
        std::vector<Card> hand;
        for (int c = 0; c < initSettings.numCards; ++c) {
            hand.push_back(ctx->drawCard(initSettings));
        }
        std::sort(hand.begin(), hand.end());
        ctx->players.push_back(hand);
    }
    
    // initialize
    ctx->playedCards.push_back(ctx->drawCard(initSettings));
    switch (ctx->topCard().rank) {
        case 2: ctx->av = 2; break;
        case 5: ctx->av = 5; break;
        case 10: ctx->mv = 1; break;
        case 11: ctx->mv = 1; ctx->cw = false; break;
        case 1: ctx->rm = false; break;
    }
    
    while (true) {
    step1:
        if (ctx->pw == false
            && std::any_of(ctx->players.begin(),
                           ctx->players.end(),
                           [](std::vector<Card> &v) { return v.size() == 0; })) {
            ccrStop(options_t{});
        }
        
        if (ctx->cw)
            ctx->activePlayer += std::max(ctx->mv, 0);
        else
            ctx->activePlayer -= std::max(ctx->mv, 0);
        while (ctx->activePlayer < 0 || ctx->activePlayer >= ctx->players.size()) {
            if (ctx->activePlayer < 0)
                ctx->activePlayer += ctx->players.size();
            else
                ctx->activePlayer -= ctx->players.size();
        }
        
        if (ctx->mv > 0)
            ctx->mv = 0;
        ++ctx->mv;
        
        ctx->fc = true;
        ctx->b = false;
        
    step2:
        ctx->options.clear();
        if (ctx->fc) {
            if (std::max(1, ctx->av) == 1)
                ctx->options.push_back({OptionType::NoCard, "Pick up 1 card"});
            else
                ctx->options.push_back({OptionType::NoCard, "Pick up " + std::to_string(std::max(1, ctx->av)) + " cards"});
        } else {
            ctx->options.push_back({OptionType::NoCard, "End turn"});
        }
        if (ctx->fc == true && ctx->av == 0) {
            ctx->options.push_back({OptionType::ReplaceHand, "Swap out your hand"});
        }
        for (Card card : ctx->players[ctx->activePlayer]) {
            bool requireMatch = ctx->rm;
            if (ctx->fc == false) {
                if (card.rank == ctx->topCard().rank)
                    requireMatch = false;
                else if (ctx->prevTopCard() != nullptr && card.rank == ctx->prevTopCard()->rank)
                    requireMatch = false;
                else if (card.rank == 3 && ctx->topCard().rank == 7 && ctx->b == true)
                    requireMatch = false;
                else if (ctx->prevTopCard() != nullptr && card.rank == 3 && ctx->prevTopCard()->rank == 7 && ctx->b == true)
                    requireMatch = false;
                else if (card.rank == 7 && ctx->topCard().rank == 3 && ctx->b == true)
                    requireMatch = false;
                else if (ctx->prevTopCard() != nullptr && card.rank == 7 && ctx->prevTopCard()->rank == 3 && ctx->b == true)
                    requireMatch = false;
                else if (ctx->playedCards.size() >= 2
                         && card.rank >= 2
                         && card.rank <= 10
                         && ctx->topCard().rank >= 2
                         && ctx->topCard().rank <= 10
                         && ctx->prevTopCard()->rank >= 2
                         && ctx->prevTopCard()->rank <= 10
                         && (card.rank + ctx->topCard().rank == ctx->prevTopCard()->rank
                             || card.rank + ctx->prevTopCard()->rank == ctx->topCard().rank
                             || ctx->topCard().rank + ctx->prevTopCard()->rank == card.rank
                             
                             || card.rank - ctx->topCard().rank == ctx->prevTopCard()->rank
                             || card.rank - ctx->prevTopCard()->rank == ctx->topCard().rank
                             || ctx->topCard().rank - ctx->prevTopCard()->rank == card.rank
                             || ctx->topCard().rank - card.rank == ctx->prevTopCard()->rank
                             || ctx->prevTopCard()->rank - card.rank == ctx->topCard().rank
                             || ctx->prevTopCard()->rank - ctx->topCard().rank == card.rank
                             
                             || card.rank * ctx->topCard().rank == ctx->prevTopCard()->rank
                             || card.rank * ctx->prevTopCard()->rank == ctx->topCard().rank
                             || ctx->topCard().rank * ctx->prevTopCard()->rank == card.rank
                             
                             || (card.rank / ctx->topCard().rank == ctx->prevTopCard()->rank && card.rank % ctx->topCard().rank == 0)
                             || (card.rank / ctx->prevTopCard()->rank == ctx->topCard().rank && card.rank % ctx->prevTopCard()->rank == 0)
                             || (ctx->topCard().rank / ctx->prevTopCard()->rank == card.rank && ctx->topCard().rank % ctx->prevTopCard()->rank == 0)
                             || (ctx->topCard().rank / card.rank == ctx->prevTopCard()->rank && ctx->topCard().rank % card.rank == 0)
                             || (ctx->prevTopCard()->rank / card.rank == ctx->topCard().rank && ctx->prevTopCard()->rank % card.rank == 0)
                             || (ctx->prevTopCard()->rank / ctx->topCard().rank == card.rank && ctx->prevTopCard()->rank % ctx->topCard().rank == 0)))
                    requireMatch = false;
                else if (card.rank == 1 && card.suit == SPADE_SUIT && (ctx->topCard().rank == 1 || ctx->topCard().rank >= 11))
                    requireMatch = false;
                else if (ctx->prevTopCard() != nullptr
                         && card.rank == 1
                         && card.suit == SPADE_SUIT
                         && (ctx->prevTopCard()->rank == 1 || ctx->prevTopCard()->rank >= 11))
                    requireMatch = false;
                else if (std::accumulate(ctx->playedCards.rbegin(),
                                            ctx->playedCards.rend(),
                                            std::pair<Card, int>{ctx->topCard(), 0},
                                            [](std::pair<Card, int> a, Card b) -> std::pair<Card, int> {
                                                if (b.rank == a.first.rank || a.first.rank == 0)
                                                    return {b, a.second + 1};
                                                return {{SPADE_SUIT, 255}, a.second};
                                            }).second == card.rank
                         && card.rank >= 2 && card.rank <= 10)
                    requireMatch = false;
                else if (card.rank + 1 == ctx->topCard().rank
                         || card.rank == ctx->topCard().rank + 1
                         || (card.rank == 1  && ctx->topCard().rank == 13)
                         || (card.rank == 13 && ctx->topCard().rank == 1))
                    ;
                else if (card.rank + 1 == ctx->prevTopCard()->rank
                         || card.rank == ctx->prevTopCard()->rank + 1
                         || (card.rank == 1  && ctx->prevTopCard()->rank == 13)
                         || (card.rank == 13 && ctx->prevTopCard()->rank == 1))
                    ;
                else
                    continue;
            }
            
            if (ctx->playedCards.size() >= 2
                && card.rank >= 2
                && card.rank <= 10
                && (card.rank
                    + ((ctx->topCard()     .rank >= 2 && ctx->topCard()     .rank <= 10) ? ctx->topCard()     .rank : 0)
                    + ((ctx->prevTopCard()->rank >= 2 && ctx->prevTopCard()->rank <= 10) ? ctx->prevTopCard()->rank : 0))
                % 3 == 0)
                continue;
            
            if (card.rank == 2 || card.rank == 5)
                if (ctx->players[ctx->activePlayer].size() > 1)
                    ctx->options.push_back({OptionType::Attack, card.desc() + " (attack)", card});
            
            if (card.rank == 3 || card.rank == 7)
                if (ctx->players[ctx->activePlayer].size() > 1 || ctx->av > 0)
                    ctx->options.push_back({OptionType::Block, card.desc() + " (block)", card});
            
            if (card.rank == 10)
                if (ctx->players[ctx->activePlayer].size() > 1)
                    if (requireMatch == false || card.suit == ctx->topCard().suit || card.rank == ctx->topCard().rank)
                        ctx->options.push_back({OptionType::ChangeMV, card.desc() + " (change mv)", card});
            
            if (card.rank == 11)
                if (ctx->players[ctx->activePlayer].size() > 1)
                    ctx->options.push_back({OptionType::ChangeDirection, card.desc() + " (change direction)", card});
            
            if (card.rank == 1)
                if (ctx->players[ctx->activePlayer].size() > 1)
                    if (ctx->av == 0)
                        ctx->options.push_back({OptionType::ChangeSuit, card.desc() + " (change suit)", card});
            
            if (ctx->av == 0)
                if (ctx->playedCards.size() < 2 || ctx->topCard().rank != 6 || ctx->prevTopCard()->rank != 6 || card.rank != 6)
                    if (requireMatch == false
                        || card.suit == ctx->topCard().suit
                        || card.rank == ctx->topCard().rank
                        || (card.rank == 12 && card.suit == HEART_SUIT && ctx->topCard().rank == 13)
                        || (card.rank == 13 && card.suit == HEART_SUIT && ctx->topCard().rank == 12)
                        || (ctx->prevTopCard() != nullptr
                            && card.rank == 1
                            && card.suit == SPADE_SUIT
                            && (ctx->prevTopCard()->rank == 1 || ctx->prevTopCard()->rank >= 11))
                        || (ctx->playedCards.size() > 0
                            && std::accumulate(ctx->playedCards.rbegin(),
                                               ctx->playedCards.rend(),
                                               std::pair<Card, int>{ctx->topCard(), 0},
                                               [](std::pair<Card, int> a, Card b) -> std::pair<Card, int> {
                                                   if (b.rank == a.first.rank || a.first.rank == 0)
                                                       return {b, a.second + 1};
                                                   return {{SPADE_SUIT, 255}, a.second};
                                               }).second == card.rank
                            && card.rank >= 2
                            && card.rank <= 10))
                        ctx->options.push_back({OptionType::NormalCard, card.desc(), card});
        }
        
        std::sort(ctx->options.begin(), ctx->options.end());
        std::unique(ctx->options.begin(), ctx->options.end());
        
        ctx->printInfo();
        
        ccrReturn(ctx->options);
        ctx->selectedOption = ctx->options[selectedOptionIdx];
        
        if (ctx->selectedOption.type != OptionType::NoCard || ctx->fc)
            ctx->rm = true, ctx->b = false, ctx->pw = false;
        
        if (ctx->selectedOption.type == OptionType::NoCard) {
            if (ctx->fc) {
                auto cardCount = std::max(1, ctx->av);
                for (auto i = 0; i < cardCount; ++i)
                    ctx->players[ctx->activePlayer].push_back(ctx->drawCard(initSettings));
                ctx->av = 0;
                ctx->pw = false;
            }
            goto step1; // go to step 1
        } else if (ctx->selectedOption.type == OptionType::ReplaceHand) {
            auto &hand = ctx->players[ctx->activePlayer];
            auto handSize = hand.size();
            for (auto i = 0; i < hand.size(); ++i) {
                ctx->playedCards.push_back(hand[i]);
            }
            hand.clear();
            for (auto i = 0; i < handSize; ++i) {
                hand.push_back(ctx->drawCard(initSettings));
            }
            goto step1;
        }
        
        ctx->playCardFromHand(ctx->selectedOption.card);
        
        if (ctx->selectedOption.type == OptionType::Attack) {
            ctx->av += ctx->selectedOption.card.rank;
        } else if (ctx->selectedOption.type == OptionType::Block) {
            ctx->av -= ctx->selectedOption.card.rank;
            if (ctx->av <= 0)
                ctx->av = 0;
            else
                ctx->b = true;
            if (ctx->topCard().rank != ctx->selectedOption.card.rank
                && (ctx->topCard().rank == 3 || ctx->topCard().rank == 7)
                && ctx->fc == false) {
                ctx->options.clear();
                ctx->options.push_back({OptionType::NormalBlockOnly, "Block normally"});
                ctx->options.push_back({OptionType::BlockEverything, "Block everything"});
                ccrReturn(ctx->options);
                ctx->selectedOption = ctx->options[selectedOptionIdx];
                if (ctx->selectedOption.type == OptionType::BlockEverything)
                    ctx->av = 0;
            }
            if (ctx->players[ctx->activePlayer].size() == 0 && ctx->av != 0)
                ctx->pw = true, ctx->mv = 0;
        } else if (ctx->selectedOption.type == OptionType::ChangeMV) {
            ctx->options.clear();
            ctx->options.push_back({OptionType::HaveAnotherTurn, "Get an extra turn"});
            ctx->options.push_back({OptionType::SkipNextPlayersTurn, "Skip the next player's turn"});
            ccrReturn(ctx->options);
            ctx->selectedOption = ctx->options[selectedOptionIdx];
            if (ctx->selectedOption.type == OptionType::HaveAnotherTurn)
                --ctx->mv;
            else
                ++ctx->mv;
        } else if (ctx->selectedOption.type == OptionType::ChangeDirection) {
            ctx->cw = !ctx->cw;
        } else if (ctx->selectedOption.type == OptionType::ChangeSuit) {
            ctx->options.clear();
            ctx->options.push_back({OptionType::ChooseSuitDiamonds, "Choose " DIAMOND_STR});
            ctx->options.push_back({OptionType::ChooseSuitHearts, "Choose " HEART_STR});
            ctx->options.push_back({OptionType::ChooseSuitSpades, "Choose " SPADE_STR});
            ctx->options.push_back({OptionType::ChooseSuitClubs, "Choose " CLUB_STR});
            ccrReturn(ctx->options);
            ctx->selectedOption = ctx->options[selectedOptionIdx];
            if (ctx->selectedOption.type == OptionType::ChooseSuitDiamonds)
                ctx->playedCards.back().suit = DIAMOND_SUIT;
            else if (ctx->selectedOption.type == OptionType::ChooseSuitHearts)
                ctx->playedCards.back().suit = HEART_SUIT;
            else if (ctx->selectedOption.type == OptionType::ChooseSuitSpades)
                ctx->playedCards.back().suit = SPADE_SUIT;
            else
                ctx->playedCards.back().suit = CLUB_SUIT;
        } else if (ctx->selectedOption.type == OptionType::NormalCard) {
            if (ctx->selectedOption.card.rank >= 2 && ctx->selectedOption.card.rank <= 10) {
                ctx->options.clear();
                ctx->options.push_back({OptionType::PlayVertically, "Play this card normally (i.e. vertically)"});
                ctx->options.push_back({OptionType::PlayHorizontally, "Play this card horizontally"});
                ccrReturn(ctx->options);
                ctx->selectedOption = ctx->options[selectedOptionIdx];
                if (ctx->selectedOption.type == OptionType::PlayHorizontally)
                    ctx->playedCards.back().rank = 11 - ctx->playedCards.back().rank;
            }
        }
        ctx->fc = false;
        goto step2;
    }
    
    ccrFinish(options_t{});
}

int main(int argc, const char *argv[]) {
    GameInitSettings settings(2, 7, std::mt19937{std::random_device{}()});
    ccrContext ctx = nullptr;
    int option = 0;
    while (true) {
        auto options = propreantepenultimate_card(&ctx, option, settings);
        if (options.size() == 0)
            break;
        std::cout << "Choose an option:\n";
        for (int i = 0; i < options.size(); ++i) {
            std::cout << "  " << (i + 1) << ": " << options[i].message << "\n";
        }
    get_input:
        std::cout << "  > ";
        int result = 0;
        std::string input;
        while (true) {
            int ci = getchar();
            if (ci == EOF) {
                std::cout << "\n";
                goto get_input;
            }
            char c = static_cast<char>(ci);
            if (c < '0' || c > '9')
                break;
            result = 10 * result + (c - '0');
        }
        if (result <= 0 || result > options.size()) {
            std::cout << "Invalid input. Please enter a number between 1 and " << options.size() << ".\n";
            goto get_input;
        }
        option = result - 1;
    }

    std::cout << "Game over.";
#ifdef _WIN32
    std::string s;
    std::cin >> s;
#endif
    return 0;
}
