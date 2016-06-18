True Propreantepenultimate Card Specification Version 3, Revision 5
-------------------------------------------------------------------

§1              Game State:
§1.1              - the current active player
                    (initially one to the left of the dealer)
§1.2              - integer AV (attack value)
                    (initially 0)
§1.3              - integer MV (move value)
                    (0)
§1.4              - boolean CW (clockwise)
                    (true)
§1.5              - boolean FC (first card this turn)
                    (true)
§1.6              - boolean B (bridging 3s and 7s)
                    (false)
§1.7              - boolean ET (end turn)
                    (false)

§2              Initial Setup:
§2.1              After cards have been dealt to the players,
                    the top card from the pile is placed on the table.
§2.2              If that card:
§2.2.1              Has rank 2 or 5: set AV to that card’s rank.
§2.2.2              Has rank 10: set MV to 1.
§2.2.3              Is a Jack: set MV to 1 and CW to false.
§2.2.4              Is an Ace: the active player must choose a suit
                      to which this card's suit changes.
§2.6              Go to step 1 (§5).

§3              Card Matches:
§3.1              Any card C can only be played:
§3.1.1              -  If the two topmost cards both have rank 6,
                         C does not have rank 6, and
§3.1.2              -  If strict matching applies,
                         requirement A is satisfied, and
§3.1.3              -  If FC is false, requirement B is satisfied
§3.2              Requirement A is satisfied if:
§3.2.1              -  If RM is false, or
§3.2.2              -  Where T is the topmost or penultimate card:
                      -  If the suit of C is equal to the suit of T
§3.3              Requirement B is satisfied if:
§3.3.1              -  Its rank is ±1 of the top or penultimate card’s rank,
                         wrapping at Ace
§3.4              Additionally, requirements A and B are both satisfied if:
§3.4.1              -  Where T is the topmost or penultimate card:
§3.4.1.1              -  If the rank of C is equal to the rank of T, or
§3.4.1.2              -  If C is a Queen of Hearts and T is any King, or
§3.4.1.3              -  If C is a King of Hearts and T is any Queen, or
§3.4.2              -  If the rank of C is equal to
                         the number of topmost consecutive cards
                         on the pile with identical ranks.
§3.4.4              -  It has rank 3 or 7,
                         and the topmost or penultimate card
                         has rank 7 or 3 respectively,
                         and B is true, or
§3.4.5              -  At least two cards have been played so far,
                         the top two cards and the card to be played
                         have a rank between 2 and 10 (inclusive),
                         and the sum or product of the ranks of
                         two of those three cards in any order is equal
                         to the remaining card’s rank

§4              Card Options:
§4.1.1            1.  If FC is true:
§4.1.1.1                Draw Max(1, AV) cards.
§4.1.1.2                Set AV to 0.
§4.1.2                Set ET to true.
§4.2.1            2.  Play a card of rank 2 or 5
                        which can’t be the player’s last card.
§4.2.2                Increase AV by the card’s rank.
§4.3.1.1          3.  Play a card of rank 3 or 7, and
§4.3.1.2                either it’s not playable as the player’s last card
§4.3.1.3                or AV mustn’t be 0.
§4.3.2                Decrease AV by this card’s rank.
§4.3.3.1              If AV is 0 or less:
§4.3.3.2                Set AV to 0.
§4.3.3.3              Otherwise:
§4.3.3.4                Set B to true.
§4.3.4.1              If the preceding card’s rank is
§4.3.4.2                different from this card’s rank,
§4.3.4.3                it is 3 or 7,
§4.3.4.4                and FC is false:
§4.3.4.5                The active player can choose to set AV to 0.
§4.3.5.1              If the active player now has no cards and AV is non-zero:
§4.3.5.2                Draw AV cards.
§4.3.5.3                Set AV to 0.
§4.3.5.4                Set ET to true.
§4.4.1            4.  Play a strictly matching card of rank 10,
                        which mustn’t be the player’s last card.
§4.4.2                The active player chooses to either
                        increase or decrease MV by 1.
§4.5.1            5.  Play a card of rank Jack
                        which is not the player’s last card.
§4.5.2                Set CW to its inverse.
§4.6.1            6.  Play a card of rank Ace
                        which is not the player’s last card,
                        and AV must be 0.
§4.6.2                The player selects a suit
                        to which this card’s suit changes.
§4.7.1            7.  Play any strictly matching card,
§4.7.2                  but AV must be 0.

§5              Step 1:
§5.1              If any player has an empty hand, that player wins the game.
§5.2              The player Max(0, MV) steps of the current active player
                    becomes the new active player, clockwise if CW is
                    true or otherwise counterclockwise.
§5.3              If MV is positive, set it to 0.
§5.4              Increment MV.
§5.5              Set FC to true and B to false.

§6              Step 2:
§6.1              Set ET to false.
§6.2              Choose any card option and follow all its instructions.
                    Numeric cards may be played horizontally.
§6.3              Horizontal played cards are turned upright, in which case
                    their rank is set to 11 minus the original rank, with 10s
                    becoming Aces.
§6.4              Set FC to false.
§6.5              If ET is true, go to step 1. Otherwise go to step 2.

§7              Final turns:
§7.1              A player can only play their last card
                    if they said the second word of this game's name
                    before the active player last changed.
§7.2              Any player who mispronounces "Propreantepenultimate Card"
                    or says "Last Card" must draw a card
                    unless the majority of players (>50%) disagree.
