True Propreantepenultimate Card Specification Version 2, Revision 3
-------------------------------------------------------------------

§1              Game State:
§1.1.1              - the current active player
§1.1.2                (initially one to the left of the dealer)
§1.2.1              - integer AV (attack value)
§1.2.2                (initially 0)
§1.3.1              - integer MV (move value)
§1.3.2                (0)
§1.4.1              - boolean CW (clockwise)
§1.4.2                (true)
§1.5.1              - boolean FC (first card this turn)
§1.5.2                (true)
§1.6.1              - boolean B (bridging 3s and 7s)
§1.6.2                (false)

§2              Dealing:
§2.1              Now the top card from the pile is placed on the table.
§2.1              If that card has rank 2 or 5, set AV to that card’s rank.
§2.2              If the rank is 10, set MV to 1.
§2.3              If the rank is Jack, set MV to 1 and CW to false.
§2.4.1            If the rank is Ace, the active player must
§2.4.2              choose a suit to which this card's suit changes.
§2.5              Go to step 1.

§3              Card Matches:
§3.1.1            Any card A can only be played (unless rules state that
§3.1.2              playing card A is possible with no restrictions):
§3.2.1              -  If the two topmost cards both have rank 6,
§3.2.2                   A must not have rank 6, and
§3.3.1              -  If A has a numeric rank
§3.3.2                   and at least two cards have been played thus far:
§3.3.3                     Set a new temporary int variable RS to the rank of A.
§3.3.4                     If the topmost card has a numeric rank:
§3.3.5                       Increase RS by the topmost card's rank.
§3.3.6                     If the penultimate card has a numeric rank:
§3.3.7                       Increase RS by the penultimate card's rank.
§3.3.8                     RS must not be divisible by three, and
§3.4                -  If FC is false, one of the following options applies:
§3.4.1                -  Where B is the topmost or penultimate card:
§3.4.1.1                -  If the rank of A is equal to the rank of B, or
§3.4.1.2                -  If A is a Queen of Hearts and B is any King, or
§3.4.1.3                -  If A is a King of Hearts and B is any Queen, or
§3.4.1.4.1              -  If A is an Ace of Spades
§3.4.1.4.2                   and B has a non-numeric rank, or
§3.4.2.1              -  If the rank of A is equal to
§3.4.2.2                   the number of topmost consecutive cards
§3.4.2.3                   on the pile with identical ranks.
§3.4.3.1              -  If strict matching does not apply:
§3.4.3.2                -  Its rank is ±1 of the top or penultimate card’s rank,
§3.4.3.3                     wrapping at Ace, or
§3.4.4.1              -  It has rank 3 or 7,
§3.4.4.2                   and the topmost or penultimate card
§3.4.4.3                   has rank 7 or 3 respectively,
§3.4.4.4                   and B is true, or
§3.4.5.1              -  At least two cards have been played so far,
§3.4.5.2                   the top two cards and the card to be played
§3.4.5.3                   have a rank between 2 and 10 (inclusive),
§3.4.5.4                   and either the
§3.4.5.5.1                   - sum
§3.4.5.5.2                   - difference
§3.4.5.5.3                   - product
§3.4.5.5.4                   - or quotient
§3.4.5.6.1                 of the ranks of two of the three cards
§3.4.5.6.2                   (top two cards from the pile
§3.4.5.6.3                   and the card to be played)
§3.4.5.7                   in any order
§3.4.5.8                   is an integer equal to the remaining card’s rank, and
§3.5.1              -  If strict matching applies:
§3.5.2                -  If FC is true:
§3.5.3                  -  If RM is false, or
§3.5.4                  -  Where B is the topmost card:
§3.5.4.1                  -  If the rank of A is equal to the rank of B, or
§3.5.4.2                  -  If the suit of A is equal to the suit of B, or
§3.5.4.3                  -  If A is a Queen of Hearts and B is any King, or
§3.5.4.4                  -  If A is a King of Hearts and B is any Queen, or
§3.5.4.5.1                -  If A is an Ace of Spades
§3.5.4.5.2                     and B has a non-numeric rank, or

§4              Card Options:
§4.1.1            1.  If FC is true:
§4.1.1.1                Draw Max(1, AV) cards
§4.1.1.2                Set AV to 0
§4.1.2                Go to step 1.
§4.2.1.1          2.  FC must be true,
§4.2.1.2                AV must be 0,
§4.2.1.3                and the player must have at least one card.
§4.2.2                Play your entire hand with no restrictions or effects.
§4.2.3                Draw as many cards as you had before.
§4.2.4                Go to step 1.
§4.3.1.1          3.  Play a card of rank 2 or 5
§4.3.1.2                which can’t be the player’s last card.
§4.3.2                Increase AV by the card’s rank.
§4.4.1.1          4.  Play a card of rank 3 or 7
§4.4.1.2.1              and either it’s not playable as the player’s last card
§4.4.1.2.2                or AV mustn’t be 0.
§4.4.2                Decrease AV by this card’s rank.
§4.4.3.1.1            If AV is 0 or less:
§4.4.3.1.2              Set AV to 0
§4.4.3.2.1            otherwise:
§4.4.3.2.2              Set B to true
§4.4.4.1.1            If the preceding card’s rank is
§4.4.4.1.2              different from this card’s rank,
§4.4.4.2                it is 3 or 7,
§4.4.4.3                and FC is false:
§4.4.4.4                The active player can choose to set AV to 0.
§4.4.5.1              If the active player now has no cards and AV is not 0:
§4.4.5.2                Draw AV cards.
§4.4.5.3                Set AV to 0.
§4.4.5.4                Go to step 1.
§4.5.1.1          5.  Play a strictly matching card of rank 10,
§4.5.1.2                which mustn’t be the player’s last card.
§4.5.2.1              The active player chooses to either
§4.5.2.2.1              - increase
§4.5.2.2.2              - or decrease
§4.5.2.3              MV by 1.
§4.6.1.1          6.  Play a card of rank Jack
§4.6.1.2                and not the player’s last card.
§4.6.2                Set CW to its inverse.
§4.7.1.1          7.  Play a card of rank Ace
§4.7.1.2                which is not the player’s last card,
§4.7.1.3                and AV must be 0.
§4.7.2.1              The player selects a suit
§4.7.2.2                to which this card’s suit changes.
§4.8.1.1          8.  Play any strictly matching card,
§4.8.1.2                but AV must be 0.
§4.8.2.1              If it has a rank between 2 and 10 (inclusive)
§4.8.2.2                and is played horizontally:
§4.8.2.3                Set its rank to 11 minus the card's original rank.

§5              Step 1:
§5.1.1            If any player has an empty hand:
§5.1.2              That player wins the game.
§5.2.1            The player Max(0, MV) steps of the current active player
§5.2.2              becomes the new active player,
§5.2.3              clockwise if CW is true or otherwise counterclockwise.
§5.3.1            If MV is positive, set it to 0.
§5.3.2            Increment MV.
§5.4              Set FC to true and B to false.

§6              Step 2:
§6.1              Choose any card option and follow all its instructions.
§6.2              Set FC to false and go to step 2.

§7              Final turns:
§7.1.1            A player can only play their last card
§7.1.2              if they said the second word of this game's name
§7.1.3              before the active player last changed.
§7.2.1            Any player who mispronounces "Propreantepenultimate Card"
§7.2.2              or says "Last Card" must draw a card
§7.2.3              unless the majority of players disagree.
