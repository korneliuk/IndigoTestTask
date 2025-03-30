#include <iostream>
#include <vector>
#include <random>
#include <time.h>

/*
You are given a locked container represented as a two-dimensional grid of boolean values (true = locked, false = unlocked).
Your task is to write an algorithm that fully unlocks the box, i.e.,
transforms the entire matrix into all false.

Implement the function:
bool openBox(uint32_t y, uint32_t x);
This function should:
	- Use the SecureBox public API (toggle, isLocked, getState).
	- Strategically toggle cells to reach a state where all elements are false.
	- Return true if the box remains locked, false if successfully unlocked.
You are not allowed to path or modify the SecureBox class.

Evaluation Criteria:
	- Functional correctness
	- Computational efficiency
	- Code quality, structure, and comments
	- Algorithmic insight and clarity
*/

class SecureBox
{
private:
	std::vector<std::vector<bool>> box;

public:

	//================================================================================
	// Constructor: SecureBox
	// Description: Initializes the secure box with a given size and 
	//              shuffles its state using a pseudo-random number generator 
	//              seeded with current time.
	//================================================================================
	SecureBox(uint32_t y, uint32_t x) : ySize(y), xSize(x)
	{
		rng.seed(time(0));
		box.resize(y);
		for (auto& it : box)
			it.resize(x);
		shuffle();
	}

	//================================================================================
	// Method: toggle
	// Description: Toggles the state at position (x, y) and also all cells in the
	//              same row above and the same column to the left of it.
	//================================================================================
	void toggle(uint32_t y, uint32_t x)
	{
		box[y][x] = !box[y][x];
		for (uint32_t i = 0; i < xSize; i++)
			box[y][i] = !box[y][i];
		for (uint32_t i = 0; i < ySize; i++)
			box[i][x] = !box[i][x];
	}

	//================================================================================
	// Method: isLocked
	// Description: Returns true if any cell 
	//              in the box is true (locked); false otherwise.
	//================================================================================
	bool isLocked()
	{
		for (uint32_t x = 0; x < xSize; x++)
			for (uint32_t y = 0; y < ySize; y++)
				if (box[y][x])
					return true;

		return false;
	}

	//================================================================================
	// Method: getState
	// Description: Returns a copy of the current state of the box.
	//================================================================================
	std::vector<std::vector<bool>> getState()
	{
		return box;
	}

private:
	std::mt19937_64 rng;
	uint32_t ySize, xSize;

	//================================================================================
	// Method: shuffle
	// Description: Randomly toggles cells in the box to 
	// create an initial locked state.
	//================================================================================
	void shuffle()
	{
		for (uint32_t t = rng() % 1000; t > 0; t--)
			toggle(rng() % ySize, rng() % xSize);
	}
};

//================================================================================
// Function: openBox
// Description: Your task is to implement this function to unlock the SecureBox.
//              Use only the public methods of SecureBox (toggle, getState, isLocked).
//              You must determine the correct sequence of toggle operations to make
//              all values in the box 'false'. The function should return false if
//              the box is successfully unlocked, or true if any cell remains locked.
//================================================================================
bool openBox(uint32_t y, uint32_t x)
{
	SecureBox box(y, x);

	// A matrix of equations that shows how toggle(y, x) operations affect the box.
	std::vector<std::vector<bool>> A(y * x, std::vector<bool>(y * x, false));
	
	// Vector for storing original state of box.
	std::vector<bool> b(y * x, false);

	// Fill in the system of equations.
	for (uint32_t row = 0; row < y; ++row)
	{
		for (uint32_t col = 0; col < x; ++col)
		{
			uint32_t idx = row * x + col;
			b[idx] = box.getState()[row][col];

			// toggle(y, x) always inverts box[y][x].
			A[idx][idx] = true;

			// Invert the elements in the current row.
			for (uint32_t i = 0; i < x; ++i)
				A[idx][row * x + i] = true;

			// Invert the elements in the current col.
			for (uint32_t i = 0; i < y; ++i)
				A[idx][i * x + col] = true;
		}
	}

	// Gauss method for Boolean equations
	for (uint32_t col = 0; col < y * x; ++col)
	{
		uint32_t pivot = col;
		while (pivot < y * x && !A[pivot][col])
			++pivot;

		// If there is no true in this column, skip it
		if (pivot == y * x)
			continue;

		std::swap(A[col], A[pivot]);
		
		bool tempB = b[col];
		b[col] = b[pivot];
		b[pivot] = tempB;

		// Turn all true to false under the pivot element.
		for (uint32_t row = col + 1; row < y * x; ++row)
		{
			// If the current element is true, then subtract (XOR) the pivot row.
			if (A[row][col])
			{
				for (uint32_t k = col; k < y * x; ++k)
					A[row][k] = A[row][k] ^ A[col][k];

				b[row] = b[row] ^ b[col];
			}
		}
	}

	// Initialize a vector of solutions xResult of size (y * x), filled with false (0). 
	// Each element of xResult[i] will contain whether toggle(i) should be executed.
	std::vector<bool> xResult(y * x, false);

	// Reverse substitution
	for (int row = y * x - 1; row >= 0; --row)
	{
		// Initial value for unknown xResult[row]
		bool sum = b[row];

		/*
		Perform an XOR operation with the already found values of xResult[col],
		where col > row (i.e., right in matrix A). 
		If A[row][col] == 1 and xResult[col] == 1, then the effect of this toggle(col)
		is added to the current equation.
		*/
		for (uint32_t col = row + 1; col < y * x; ++col)
			sum ^= (A[row][col] & xResult[col]);

		xResult[row] = sum;
	}

	// Apply the found operations to SecureBox
	for (uint32_t i = 0; i < y * x; ++i)
	{
		/*
		Call the toggle method for the corresponding row and column. 
		The row is defined as i / x (block number in the matrix). 
		The column is defined as i % x (position within the block).
		*/
		if (xResult[i])
			box.toggle(i / x, i % x);
	}

	return box.isLocked();
}


int main(int argc, char* argv[])
{
	uint32_t y = std::atol(argv[1]);
	uint32_t x = std::atol(argv[2]);
	bool state = openBox(y, x);

	if (state)
		std::cout << "BOX: LOCKED!" << std::endl;
	else
		std::cout << "BOX: OPENED!" << std::endl;

	return state;
}

