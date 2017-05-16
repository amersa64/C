/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direcounter mapped cache with a block size of 32 bytes.
 */

#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_32_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_64_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_61_67(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  switch (M) {
    case 32:
    transpose_32_32(M, N, A, B);
    break;
    case 64:
    transpose_64_64(M, N, A, B);
    break;
    case 61:
    transpose_61_67(M, N, A, B);
    break;
  }
}

/*
* You can define additional transpose functions below. We've defined
* a simple one below to help you get started.
*/
// The sizeof(int) == 4 and the block size is 2^5 or 32 per line,
// meaning each line can hold 8 ints. Using blocking, the block-size of
// 8 nicely divides the 32 square matrix test case
char transpose_32_32_desc[] = "Transpose the 32 x 32 matrix";
void transpose_32_32(int M, int N, int A[N][M], int B[M][N]) {
  int rowIndex; // Index to pass through the rows
  int colIndex; // Index to pass through the columns
  int blockedRowIndex; // Index to pass through the rows in a block
  int blockedColIndex; // Index to pass through the cols in a block
  // Later, realizing that a block's diagonal need not be affected if it is
  // part of the matrix's entire diagonal, use temporary local variables
  // instead of having to re-access those elements
  int eBlockDiagl; // Element of the diagonal
  int iBlockDiagl; // Index of the diagonal (row/column index)
  // Even though M == N, both will be used for intuitive clarity
  for (colIndex = 0; colIndex < M; colIndex += 8) {
    for (rowIndex = 0; rowIndex < N; rowIndex +=8) {
      for (blockedRowIndex = rowIndex; blockedRowIndex < rowIndex +
        8; ++blockedRowIndex) {
          for (blockedColIndex = colIndex; blockedColIndex < colIndex
            + 8; ++blockedColIndex) {
              if (blockedRowIndex != blockedColIndex) {
                B[blockedColIndex][blockedRowIndex] =
                A[blockedRowIndex][blockedColIndex];
              }
              else {
                eBlockDiagl = A[blockedRowIndex][blockedColIndex];
                iBlockDiagl = blockedRowIndex;
              }
            }
            // If it's on the diagonal, refer to the diagonal stored while
            // iterating through each row in the block
            if (colIndex == rowIndex) {
              B[iBlockDiagl][iBlockDiagl] = eBlockDiagl;
            }
          }
        }
      }
    }

    char transpose_64_64_desc[] = "Transpose the 64 x 64 matrix";
    void transpose_64_64(int M, int N, int A[N][M], int B[M][N]) {
      /*
      int rowIndex, colIndex, blockedRowIndex, blockedColIndex,diag1, diag2,diag3, diag4;
      int reg1, reg2, reg3 , reg4;
      for(rowIndex=0;rowIndex<M;rowIndex+=8){
        for(colIndex=0;colIndex<N;colIndex+=8){
          for(blockedRowIndex=rowIndex;blockedRowIndex<rowIndex+8;blockedRowIndex++){
            for(blockedColIndex=colIndex;blockedColIndex<colIndex+8;blockedColIndex+=4){
              if(blockedRowIndex!=blockedColIndex){
                reg1=A[blockedRowIndex][blockedColIndex];
                reg2=A[blockedRowIndex][blockedColIndex+1];
                reg3=A[blockedRowIndex][blockedColIndex+2];
                reg4=A[blockedRowIndex][blockedColIndex+3];
                B[blockedColIndex][blockedRowIndex]=reg1;
                B[blockedColIndex+1][blockedRowIndex]=reg2;
                B[blockedColIndex+2][blockedRowIndex]=reg3;
                B[blockedColIndex+3][blockedRowIndex]=reg4;
              }
              else{
                switch (blockedRowIndex % 4) {
                case 0:{diag1=A[blockedRowIndex][blockedColIndex];B[blockedRowIndex][blockedColIndex]=diag1;};break;
                case 1:{diag2=A[blockedRowIndex][blockedColIndex];B[blockedRowIndex][blockedColIndex]=diag2;};break;
                case 2:{diag3=A[blockedRowIndex][blockedColIndex];B[blockedRowIndex][blockedColIndex]=diag3;};break;
                case 3:{diag4=A[blockedRowIndex][blockedColIndex];B[blockedRowIndex][blockedColIndex]=diag4;};break;
              }
            }
          }
        }
      }
    }
  }
  */
    int rowIndex, colIndex;
    int reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8;
    int counter; // Since only 12 local variables are allowed, 8 is used as a magic #
    for (rowIndex = 0; rowIndex < N; rowIndex += 8) {
        for (colIndex = 0; colIndex < M; colIndex += 8) {
            // Now operating block-wise
            for (counter = 0; counter < 8; counter++){
                // Utilize the local variables as "registers" to avoid
                // future misses
                // Store the 4 numbers in the current row in A
                //using these registers reduces the misses by 200
                //since they are already cached but if we had to load them from
                //memory this would cause misses.
                reg1 = A[colIndex + counter][rowIndex];
                reg2 = A[colIndex + counter][rowIndex + 1];
                reg3 = A[colIndex + counter][rowIndex + 2];
                reg4 = A[colIndex + counter][rowIndex + 3];
                if (0 == counter) {
                    // If on the first iteration within a block,
                    // Store the start of each row
                    reg5 = A[colIndex][rowIndex + 4];
                    reg6 = A[colIndex][rowIndex + 5];
                    reg7 = A[colIndex][rowIndex + 6];
                    reg8 = A[colIndex][rowIndex + 7];
                }
                // Now consider the block in B to be transposed into
                // Advancing in 64s (jump across an entire row), swap
                B[rowIndex][colIndex + counter] = reg1;
                B[rowIndex+1][colIndex + counter] = reg2;
                B[rowIndex+2][colIndex + counter] = reg3;
                B[rowIndex+3][colIndex + counter] = reg4;
            }
            // Now go down (note how there's only 7 iterations since there is a
            // corner block shared in row-wise and column-wise iterations
            for (counter = 7; counter > 0; counter--) {
                reg1 = A[colIndex + counter][rowIndex + 4];
                reg2 = A[colIndex + counter][rowIndex + 5];
                reg3 = A[colIndex + counter][rowIndex + 6];
                reg4 = A[colIndex + counter][rowIndex + 7];
                B[rowIndex + 4][colIndex + counter] = reg1;
                B[rowIndex + 5][colIndex + counter ] = reg2;
                B[rowIndex + 6][colIndex + counter ] = reg3;
                B[rowIndex + 7][colIndex + counter ] = reg4;
            }
            // Consider the block in B to be transposed into
            B[rowIndex + 4][colIndex] = reg5;
            B[rowIndex + 5][colIndex] = reg6;
            B[rowIndex + 6][colIndex] = reg7;
            B[rowIndex + 7][colIndex] = reg8;
        }
    }
}
char transpose_61_67_desc[] = "Transpose the 61 x 67 matrix";
void transpose_61_67(int M, int N, int A[N][M], int B[M][N]) {
    int rowIndex, colIndex; // Indices to pass through rows/columns in blocks
    int blockedRowIndex, blockedColIndex; // Indices to step within blocks
    int temp;
    for(rowIndex = 0; rowIndex < N; rowIndex += 16) {
        for(colIndex = 0; colIndex < M; colIndex += 4) {
            // Do not try to transpose outside of the bounds of the matrix
            for(blockedRowIndex = rowIndex; (blockedRowIndex < rowIndex + 16) &&
                (blockedRowIndex < N); ++blockedRowIndex) {
                for(blockedColIndex = colIndex; (blockedColIndex < colIndex + 4)
                    && (blockedColIndex < M); ++blockedColIndex) {
                    // Store diagonals
                    if (blockedRowIndex - rowIndex ==
                        blockedColIndex - colIndex) {
                        temp = A[blockedRowIndex][blockedColIndex];
                    }
                    else { // Do a normal in-block swap
                        B[blockedColIndex][blockedRowIndex] =
                            A[blockedRowIndex][blockedColIndex];
                    }
                }
                for (blockedColIndex = colIndex; (blockedColIndex <colIndex + 4)
                 && (blockedColIndex < M); ++blockedColIndex) {
                    // Map the diagonals
                    if (blockedRowIndex - rowIndex ==
                        blockedColIndex - colIndex) {
                        B[blockedColIndex][blockedRowIndex] = temp;
                    }
                }
            }
        }
    }
}

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple r w-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerfunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
   //registerTransfunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correcounterness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
// Citation: This code was delivered after extensive research and I fully
// understand it, please contact Amer Alsabbagh for referrences. Thanks
