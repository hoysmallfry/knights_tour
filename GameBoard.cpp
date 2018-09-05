/******************************************************************************/
/*!
\file   Gameboard.cpp
\author Joel Barba
\par    email: jbarba\@digipen.edu
\par    DigiPen login: jbarba
\par    Course: CS280
\par    Assignment #3
\date   10/20/2009
\brief
This is the implementation file for all member functions
of the classes GameBoard, Search, and Space.

*/
/******************************************************************************/

#include "GameBoard.h"
#include <math.h>

//No magic numbers in my house.
const int longWay = 2;
const int shortWay = 1;
const unsigned numMoves = 8;

/******************************************************************************/
/*!

Defaults the coordinates to 0,0.

*/
/******************************************************************************/

Space::Space( void )
:	row_(0), column_(0) {}

/******************************************************************************/
/*!

Assign coordinates of a space on the board.

\param row
Assigns the row coordinate.

\param column
Assigns the column coordinate.

*/
/******************************************************************************/

Space::Space( const unsigned& row, const unsigned& column )
:	row_(row), column_(column) {}

/******************************************************************************/
/*!

Copies a Space.

\param copy
The Space to be copied.

*/
/******************************************************************************/

Space::Space( const Space& copy )
:	row_(copy.row_), column_(copy.column_) {}

/******************************************************************************/
/*!

Assign coordinates of a space.

\param row
Assigns the row coordinate.

\param column
Assigns the column coordinate.

*/
/******************************************************************************/

void Space::operator()( const unsigned& row, const unsigned& column )
{
	row_ = row; 
	column_ = column;
}

/******************************************************************************/
/*!

Retrieves a read-only reference of the row_ member.

\return
The row coordinate.

*/
/******************************************************************************/

const unsigned& Space::getRow( void )const
{
	return row_;
}

/******************************************************************************/
/*!

Retrieves a read-only reference of the column_ member.

\return
The column coordinate.

*/
/******************************************************************************/

const unsigned& Space::getColumn( void )const
{
	return column_;
}

/******************************************************************************/
/*!

Reassigns a Space.

\param rhs
The Space to copy members from.

\return
reference to this class.

*/
/******************************************************************************/
const Space& Space::operator=( const Space& rhs )
{
	row_ = rhs.row_;
	column_ = rhs.column_;

	return *this;
}

/******************************************************************************/
/*!

Constructs an instance of a Search.

\param gameboard
An pointer to the gameboard.

*/
/******************************************************************************/
Search::Search( const GameBoard* gameboard )
: gameboard_(gameboard), leftHasPriority(false), rightHasPriority(true) {}

/******************************************************************************/
/*!

Compares two Spaces based on the gameboard_ policy_ member.

\param lhs
The left hand Space to compare.

\param rhs
The right hand Space to compare.

\return
Which element to prioritize.

*/
/******************************************************************************/
bool Search::operator()( const Space& lhs, const Space& rhs ) const
{
	//arranges them reverse than how they are fed.
	if( gameboard_->GetTourPolicy() == GameBoard::tpSTATIC ) return leftHasPriority;

	const unsigned leftIndex = gameboard_->get1DIndex( lhs.getRow(), lhs.getColumn() );
	const unsigned rightIndex = gameboard_->get1DIndex( rhs.getRow(), rhs.getColumn() );

	const int* heuristicsBoard = gameboard_->GetHTable();
	const int leftHeuristic = heuristicsBoard[leftIndex];
	const int rightHeuristic = heuristicsBoard[rightIndex];

	const double* distanceBoard = gameboard_->GetDTable();
	const double leftDistance = distanceBoard[leftIndex];
	const double rightDistance = distanceBoard[rightIndex];

	//If heuristic value is the same, then check distance value.
	if( leftHeuristic==rightHeuristic )
	{
		//If distance of left is greater or equal, left has priority.
		if( leftDistance >= rightDistance )
			return leftHasPriority;
		//Otherwise, right has priority.
		return rightHasPriority;
	}
	//Otherwise, if left side has a lower heuristic, left has priority.
	else if( leftHeuristic<rightHeuristic )
		return leftHasPriority;

	//Otherwise, right side has priority.
	return rightHasPriority;
}

/******************************************************************************/
/*!

Constructs an instance of a Gameboard.

\param rows
The total number of rows in the board.

\param columns
The total number of columns in the board.

\param callback
A function to call suring the KnightsTour() function.

*/
/******************************************************************************/
GameBoard::GameBoard(unsigned rows, unsigned columns, KNIGHTS_CALLBACK callback)
:	rows_(rows), columns_(columns), callback_(callback), message_(MSG_PLACING)
{
	size_ = rows_*columns_ ;
	setDistanceBoard();
}

/******************************************************************************/
/*!

Clears the three boards.

*/
/******************************************************************************/
GameBoard::~GameBoard( void )
{
	moveBoard_.clear();
	heuristicsBoard_.clear();
	distanceBoard_.clear();
}

/******************************************************************************/
/*!

Constructs an instance of a Gameboard.

\param row
The row coordinate of the space given.

\param column
The column coordinate of the space given.

\param policy
A type of search to perform.

*/
/******************************************************************************/
bool GameBoard::KnightsTour(unsigned row, unsigned column, TourPolicy policy)
{
	//resets the boards on first iteration and sets the policy member.

	//resets total moves performed 
	totalMoves_ = 0;
	//resets to move 1, the first iteration.
	iteration_ = 1;
	//sets the determinant policy
	policy_ = policy;
	//The first piece is placed down.
	message_ = MSG_PLACING;

	//resets the movement board.
	setMoveBoard();
	//resets the heuristics board.
	setHeuristicsBoard();

	//starts the tour, retrives the result.
	bool tour = PlaceKnight( row, column );

	//call the callback function to see what the final status was.
	callback_( *this, &moveBoard_[0], message_, totalMoves_, rows_, columns_, currentSpace.getRow(), currentSpace.getColumn() );

	//If queue is empty, but no end has been reached, return false.
	return tour;
}

bool GameBoard::PlaceKnight( const unsigned& row, const unsigned& column )
{
	//increases the move counter.
	++totalMoves_;

	//Get the 1-D index of the 2-D coordinates.
	unsigned index = get1DIndex( row, column );

	//Set the piece on the movement board.
	moveBoard_[index] = iteration_;

	//have all the spots on the board been reached?
	if( isSolved() )
	{
		//set the callback message.
		message_ = MSG_FINISHED_OK;
		return true;
	}

	//get the next available moves.
	MoveContainer nextMoves = getNextAvailable( row, column );

	//while the queue is not empty,
	while( !nextMoves.empty() )
	{
		//increment current move
		++iteration_;

		//take the top and call this function.
		currentSpace = nextMoves.top();

		//call the callback function.
		callback_( *this, &moveBoard_[0], message_, totalMoves_, rows_, columns_, row, column );

		message_ = MSG_PLACING;

		if( PlaceKnight(currentSpace.getRow(), currentSpace.getColumn() ) )
		{
			//if iteration returns true, return true.
			return true;
		}
		//otherwise, move was a dead end. Undo some operations.
		else
		{
			//pop the last space off the stack
			nextMoves.pop();

			//decrement current move
			--iteration_;

			message_ = MSG_REMOVING;

			//get the index of the space that was last used.
			const unsigned nextIndex = get1DIndex( currentSpace.getRow(), currentSpace.getColumn());
			//remove the move off the movement board.
			moveBoard_[nextIndex] = 0;
			//reset the wrong space's heuristic. 
			++heuristicsBoard_[nextIndex];
		}

	}

	//The queue is empty and the board is not solved. This is a dead end.
	message_ = MSG_FINISHED_FAIL;
	return false;
}

/******************************************************************************/
/*!

Returns the number of moves performed

\return
The number of total moves performed.

*/
/******************************************************************************/
unsigned GameBoard::GetMoves(void) const        // the number of moves made
{
	return totalMoves_;
}

/******************************************************************************/
/*!

Returns the TourPolicy member

\return
The type of search being used.

*/
/******************************************************************************/
GameBoard::TourPolicy GameBoard::GetTourPolicy(void) const // the policy used to search
{
	return policy_;
}

/******************************************************************************/
/*!

Returns the Movement board

\return
The movement board.

*/
/******************************************************************************/
int const *GameBoard::GetBoard(void) const
{
	return &moveBoard_[0];
}

/******************************************************************************/
/*!

Returns the heuristics board

\return
The heuristics board.

*/
/******************************************************************************/
int const *GameBoard::GetHTable(void) const
{
	return &heuristicsBoard_[0];
}

/******************************************************************************/
/*!

Returns the distance board

\return
The distance board.

*/
/******************************************************************************/
double const *GameBoard::GetDTable(void) const // 1-D representation of distance table
{
	return &distanceBoard_[0];
}


/******************************************************************************/
/*!

Sets the values in the movement board to 0.

*/
/******************************************************************************/
void GameBoard::setMoveBoard( void )
{
	moveBoard_.clear();
	moveBoard_.reserve( size_);

	for( unsigned i=0; i<size_; i++ )
	{
		moveBoard_.push_back( 0 );
	}
}

/******************************************************************************/
/*!

Sets the values in the heuristics board based on its size.

*/
/******************************************************************************/
void GameBoard::setHeuristicsBoard( void )
{
	heuristicsBoard_.clear();
	heuristicsBoard_.reserve( size_ );

	const unsigned options[3][3] = {{ 2, 3, 4 },
									{ 3, 4, 6 },
									{ 4, 6, 8 }};

	const unsigned* optionRow;
	const unsigned* option;

	for( unsigned i=0; i<rows_; i++ )
	{
		if( i==0 || i==(rows_-1) )
			optionRow = &options[0][0];	
		else if( i==1 || i==(rows_-2) )
			optionRow = &options[1][0];
		else
			optionRow = &options[2][0];

		for( unsigned j=0; j<columns_; j++ )
		{
			if( j==0 || j==(columns_-1) )
				option = optionRow;	
			else if( j==1 || j==(columns_-2) )
				option = (optionRow+1);
			else
				option = (optionRow+2);

			heuristicsBoard_.push_back( *option );
		}
	}
}

/******************************************************************************/
/*!

Sets the values in the distance board based on a positions distance from center.

*/
/******************************************************************************/
void GameBoard::setDistanceBoard( void )
{
	distanceBoard_.clear();
	distanceBoard_.reserve( size_ );

	for( unsigned i=0; i<rows_; i++ )
	{
		const double y = static_cast<int>(i) - (static_cast<double>(rows_-1)/2.0);

		for( unsigned j=0; j<columns_; j++ )
		{
			const double x = static_cast<int>(j) - (static_cast<double>(columns_-1)/2.0);

			const double distance = sqrt( (x*x) + (y*y) );
			distanceBoard_.push_back( distance );
		}
	}
}

/******************************************************************************/
/*!

Finds the next available spaces, adds them if available, and sorts them based on policy.

\param row
The row coordinate of the space given.

\param column
The column coordinate of the space given.

\return
The next moves possible in their order.

*/
/******************************************************************************/
const GameBoard::MoveContainer GameBoard::getNextAvailable( const unsigned& row, const unsigned& column )
{
	  
	Search search( this );
	MoveContainer nextMoves( search );

	// Holds the displacement values.
	const int rJump[] = { -shortWay, -longWay, -longWay, -shortWay, +shortWay, +longWay, +longWay, +shortWay };
	const int cJump[] = { +longWay, +shortWay, -shortWay, -longWay, -longWay, -shortWay, +shortWay, +longWay };

	for( unsigned i=0; i<numMoves; i++ )
	{
		Space space( row+rJump[i], column+cJump[i] );
		//If the space is available on the board, we place it on the nextMove queue.
		if( isAvailable(space) )
		{
			//decrements the element in the heuristics board
			const unsigned index = get1DIndex( space.getRow(), space.getColumn() );
			--heuristicsBoard_[index];

			//pushes the space onto the queue.
			nextMoves.push( space );
		}
	}

	return nextMoves;
}

/******************************************************************************/
/*!

Finds the 1-D dimensional index number of a space in relation to a vector.

\param row
The row coordinate of the space given.

\param column
The column coordinate of the space given.

\return
The index of the array

*/
/******************************************************************************/
unsigned GameBoard::get1DIndex( const unsigned& row, const unsigned& column ) const
{
	return (row*columns_)+column;
}

/******************************************************************************/
/*!

Determines if a Space given is in bounds and has not been already used..

\param space
The space to check

\return
If the space is available or not

*/
/******************************************************************************/
bool GameBoard::isAvailable( const Space& space ) const
{
	const unsigned row = space.getRow();
	const unsigned column = space.getColumn();

	//checks if position is out of bounds.
	if( row >= rows_ )
		return false;
	if( column >= columns_ )
		return false;

	//checks if the position has already been reached.
	const unsigned index = get1DIndex( row, column );

	//returns the opposite of the move board status at the index.
	return moveBoard_[index] == 0;
}

/******************************************************************************/
/*!

Runs through the movement board and determines if all spaces have been reached.

\return
If the board has been solved or not

*/
/******************************************************************************/
bool GameBoard::isSolved( void ) const
{
	//Iterate through the spots on the board.
	for( unsigned i=0; i< moveBoard_.size(); i++ )
	{
		//If not all spots have been reached, return false.
		if( !moveBoard_[i] ) return false;
	}

	//All spots have been reached.
	return true;
}
