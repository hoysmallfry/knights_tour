/******************************************************************************/
/*!
\file   Gameboard.h
\author Joel Barba
\par    email: jbarba\@digipen.edu
\par    DigiPen login: jbarba
\par    Course: CS280
\par    Assignment #3
\date   10/20/2009
\brief
This is the declaration file for all member functions
of the classes GameBoard, Search, and Space.

*/
/******************************************************************************/

//---------------------------------------------------------------------------
#ifndef GAMEBOARDH
#define GAMEBOARDH
//---------------------------------------------------------------------------

#include <vector>
#include <queue>

// Represents a space on the board.
struct Space
{
private:
	unsigned row_;
	unsigned column_;
public:
	Space( void );
	Space( const unsigned& row, const unsigned& column );
	Space( const Space& copy );

	void operator()( const unsigned& row, const unsigned& column );
	const unsigned& getRow( void )const;
	const unsigned& getColumn( void )const;

	const Space& operator=( const Space& rhs );
};

// Forward declaration
class Search;

class GameBoard
{
  public:
    enum BoardMessage 
    {
      MSG_FINISHED_OK,   // finished and found a tour
      MSG_FINISHED_FAIL, // finished but no tour found
      MSG_ABORT_CHECK,   // checking to see if algorithm should continue
      MSG_PLACING,       // placing a knight on the board
      MSG_REMOVING       // removing a knight (back-tracking)
    };

    typedef bool (*KNIGHTS_CALLBACK)
      (const GameBoard& gb,  // the gameboard object itself
       const int *board,     // one-dimensional array
       BoardMessage message, // type of message
       unsigned move,        // the move number
       unsigned rows,        // number of rows in the board
       unsigned columns,     // number of columns in the board
       unsigned row,         // current row algorithm is accessing
       unsigned column       // current column algorithm is accessing
      );

    enum TourPolicy 
    {
      tpSTATIC,    // use a fixed set of offsets for next move
      tpHEURISTICS // use heuristics for next move
    };

    // Constructor/Destructor
    GameBoard(unsigned rows, unsigned columns, KNIGHTS_CALLBACK callback = 0);
    ~GameBoard();

      // Starts the tour at row,column using specified tour policy
    bool KnightsTour(unsigned row, unsigned column, TourPolicy policy = tpSTATIC);
    unsigned GetMoves(void) const;        // the number of moves made
    TourPolicy GetTourPolicy(void) const; // the policy used to search
    int const *GetBoard(void) const;      // 1-D representation of board state

      // Debugging helpers
    int const *GetHTable(void) const;    // 1-D representation of heuristic table
    double const *GetDTable(void) const; // 1-D representation of distance table

	//takes the 2-D coordinates and returns a 1-D index.
	unsigned get1DIndex( const unsigned& row, const unsigned& column ) const;

  private:
    unsigned rows_;
    unsigned columns_;
    KNIGHTS_CALLBACK callback_;
    TourPolicy policy_;
    
    // Other private fields and methods ...
	typedef std::priority_queue< Space, std::vector<Space>, Search > MoveContainer;
	
	//keeps track of the total moves
	unsigned totalMoves_;
	//keeps track of the current move.
	int iteration_;
	//knows the size of the arrays.
	unsigned size_;
	//keeps track of the current board message.
	BoardMessage message_;
	//keeps track of the latest spot.
	Space currentSpace;

	//The boards.
	std::vector<int> heuristicsBoard_;
	std::vector<double> distanceBoard_;
	std::vector<int> moveBoard_;

	//recursive call.
	bool PlaceKnight( const unsigned& row, const unsigned& column );

	//Sets the naive board
	void setMoveBoard( void );
	void setHeuristicsBoard( void );
	void setDistanceBoard( void );

	//returns true if the place at row,column can be landed on.
	bool isAvailable( const Space& space ) const;

	//returns all the next available positions on the board.
	const MoveContainer getNextAvailable( const unsigned& row, const unsigned& column );

	//checks the board to see if all spots on the board have been reached.
	bool isSolved( void ) const;

};

class Search
{
private:
	const GameBoard* gameboard_;
	const bool leftHasPriority;
	const bool rightHasPriority;

public:
	Search( const GameBoard* gameboard );
	bool operator()( const Space& lhs, const Space& rhs ) const;
	const Search& operator=( const Search& rhs );
};

#endif  // GAMEBOARDH
