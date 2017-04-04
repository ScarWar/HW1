#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


#define report_error(str) printf("LINE: %d, Error - %s %s\n", __LINE__, (str), strerror(errno))
#define min(x, y) ((x) <= (y) ? (x) : (y))

// Error messages
#define ARGC_ERR_MSG "Invalid nubmer of argument. Expected 4, but recieved"
#define FORMANT_ERR_MSG "Invalid argument foramt."
#define ALLOC_ERR_MSG "Memory allocation failed."
#define READ_FILE_ERR_MSG "Failed reading."
#define WRITE_FILE_ERR_MSG "Failed writing."
#define OPEN_FILE_ERR_MSG "Unable to open file."
#define CREATE_FILE_ERR_MSG "Unable to create file."
#define NULL_PINTER_ERR_MSG "Trying to use null pointer"


typedef enum data_unit_t {
    B,
    K,
    M,
    G
} DataUnit;


typedef struct data_t {
    int amount;
    DataUnit dataUnit;
} DataAmount;


/**
 * Return amount of data in kilobyte
 * @param data - Data amount
 * @return amount of data in kilobyte
 */
long long getAmountKiloByte(DataAmount data) {
    if (data.dataUnit == G) {
        return data.amount * 1073741824;
    } else if (data.dataUnit == M) {
        return data.amount * 1048576;
    } else if (data.dataUnit == K) {
        return data.amount * 1024;
    } else {
        return data.amount;
    }
}

/**
 *
 * @param str - Input data
 * @return The amount of kilobytes requested
 */
long long getDataAmount(char *str) {
    char *tmp;
    int res = (int) strtol(str, &tmp, 10);

    DataAmount data;
    data.amount = res;
    if (0 == strcmp(tmp, "B")) {
        data.dataUnit = B;
    } else if (0 == strcmp(tmp, "K")) {
        data.dataUnit = K;
    } else if (0 == strcmp(tmp, "M")) {
        data.dataUnit = M;
    } else if (0 == strcmp(tmp, "G")){
        data.dataUnit = G;
    } else {
        return -1;
    }
    return getAmountKiloByte(data);
}

/**
 * Return if the value of the byte is between 32 and 126 
 * @param c - byte
 * @return  - 1 if the byte value is between 32 and 126,
		 	  otherwise 0.
 */
int isPrintable(char c) {
    return 32 <= c && c <= 126;
}

/**
 * Return the size of the buffer which fits to the data amount
 * requested by user. Using convention that
 * - 1028B = K
 * - 1028K = M
 * - 1028M = G
 * data amount received by user
 * @param data - Amount of data to read
 * @return - integer - buffer size
 */
int getBufferSize(long long int amount) {
    if (amount >= 1048576) { // 1024 ^ 2 = 1048576 aka over a 1M
        return 4096;
    }
    return 1024;
}


/**
 * Print statistic in nice format
 * @param charReq 		- Number of characters requested
 * @param charRead 		- Number of characters read
 * @param charPrintable - Number of printable characters
 */
int printStatistics(long long int charReq, long long int charRead, long long int charPrintable) {
	if( charReq < 0 || charRead < 0 || charPrintable < 0){
        printf("LINE: %d, Error - %s\n", __LINE__, "Can't recieve negative numbers");
        return 0;
    }
    printf("%lld characters requested, %lld characters read, %lld are printable\n", charReq, charRead, charPrintable);
    return 1;
}

/**
 *
 * @param inputBuffer   - String to filter
 * @param outputBuffer  - String to which filtered string is copied
 * @param bufferSize    - Size of the buffer
 * @param readNumber    - How much to from string
 * @param iBufferIndex  - The index from which the input string have a relevant data
 * @param oBufferIndex  - The index from which I start copy to outputBuffer
 * @param isBufferFull  - A flag to know if the outputBuffer is full
 * @return The number of bytes which are printable
 */
int
filterBuffer(char *inputBuffer, char *outputBuffer, int bufferSize, int readNumber, int *iBufferIndex, int oBufferIndex,
             char *isBufferFull) {
    if (inputBuffer == NULL || outputBuffer == NULL) {
        printf("LINE: %d, Error - %s\n", __LINE__, NULL_PINTER_ERR_MSG);
        return -1;
    }

    if (bufferSize < 0 || oBufferIndex < 0 || *iBufferIndex < 0 || readNumber < 0) {
        printf("LINE: %d, Error - %s\n", __LINE__, "Can't recieve negative numbers");
        return -1;
    }
    int i, j = 0;
    for (i = *iBufferIndex; i < readNumber; ++i) {
        if (isPrintable(inputBuffer[i])) {
            outputBuffer[oBufferIndex + j++] = inputBuffer[i];
        }
        // Write to output file If file outputBuffer is full.
        // Set the index from which you should filter start
        // filter.
        if (oBufferIndex + j == bufferSize) {
            *isBufferFull = 1;
            *iBufferIndex = ++i;
            return j;
        }
    }
    return j;
}

/**
 *
 * @param outputBuffer  - Buffer from which to write
 * @param ofd           - Output file descriptor
 * @param size          - The size of bytes to write
 * @param output_file   - The name of the file to write
 * @return 1 if the writing is successful, 0 otherwise
 */
char writeBuffer(char *outputBuffer, int ofd, int size, char *output_file) {
    if (outputBuffer == NULL || output_file == NULL) {
        printf("LINE: %d, Error - %s\n", __LINE__, NULL_PINTER_ERR_MSG);
        return 0;
    }
    if (size < 0) {
        printf("LINE: %d, Error - %s\n", __LINE__, "Invalid argument, can't recieve negative values");
        return 0;
    }
    if(size == 0)
        return 1;
    ssize_t written = write(ofd, outputBuffer, (size_t) size);
    if (written < 0) {
        report_error(WRITE_FILE_ERR_MSG);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    int bufferSize, bufferReadSize = 0, bufferWriteSize = 0;
    char *inputBuffer = NULL, *outputBuffer = NULL;
    long long int outputSize, charReq = 0;

    // Data amount, input, output
    char *data, *input_file, *output_file;

    // Input file descriptor, output file descriptor
    int ifd = 0, ofd = 0;

    char isBufferFull = 0;
    int iBufferIndex = 0, oBufferIndex = 0, printable = 0;
    ssize_t readNumber, writtenCount = 0, printableCount = 0;

    if (argc != 4) {
        printf("LINE: %d, Error - %s %d\n", __LINE__, ARGC_ERR_MSG, argc);
        return 0;
    }

    data = argv[1];         // Data amount
    input_file = argv[2];   // Input file name
    output_file = argv[3];  // Output file name

    outputSize = getDataAmount(data);
    if(outputSize == -1){
        report_error(FORMANT_ERR_MSG);
        return -1;
    }
    bufferSize = getBufferSize(outputSize);
    charReq = outputSize;


    inputBuffer = malloc(bufferSize * sizeof(char));
    outputBuffer = malloc(bufferSize * sizeof(char));

    if (inputBuffer == NULL || outputBuffer == NULL) {
        report_error(ALLOC_ERR_MSG);
        return -1;
    }

    ifd = open(input_file, O_RDONLY, S_IRUSR);
    if (ifd == -1) {
        report_error(OPEN_FILE_ERR_MSG);
        return -1;
    }

    // Create new file with User premission to read write 
    ofd = creat(output_file, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if (ofd == -1) {
        report_error(CREATE_FILE_ERR_MSG);
    	close(ofd);
        return -1;
    }

    ofd = open(output_file, O_WRONLY, S_IWUSR);
    if (ofd == -1) {
        report_error(OPEN_FILE_ERR_MSG);
        close(ofd);
		close(ifd);
        return -1;
    }

    while (outputSize > 0) {
        bufferReadSize = (int) min(outputSize, bufferSize);

        readNumber = read(ifd, inputBuffer, (size_t) bufferReadSize);

        if (readNumber < 0) {
            report_error(READ_FILE_ERR_MSG);
		    close(ofd);
    		close(ifd);	
            return -1;
        }

        if (readNumber == 0) {
            // EOF, Reset pointer if we read all the file 
            if(lseek(ifd, 0, SEEK_SET) == -1){
                printf("LINE: %d, Error - %s\n", __LINE__, strerror(errno));
                close(ofd);
                close(ifd); 
                return -1;
            }
        } else {
            printable = filterBuffer(inputBuffer, outputBuffer, bufferSize, (int) readNumber, &iBufferIndex, oBufferIndex, &isBufferFull);
            if (printable == -1) {
				close(ofd);
    			close(ifd);
                return -1;
            }

            if (isBufferFull) {

                // How much printable bytes
                printableCount += printable;

                // Write to output
                if (!writeBuffer(outputBuffer, ofd, bufferSize, output_file)) {
				    close(ofd);
    				close(ifd);
                    return -1;
                }

                // Fill the output buffer with bytes left in input buffer
                oBufferIndex = 0;
                printable = filterBuffer(inputBuffer, outputBuffer, bufferSize, (int) readNumber, &iBufferIndex, oBufferIndex, &isBufferFull);

                if (printable == -1) {
                    close(ofd);
                    close(ifd);
                    return -1;
                }

                iBufferIndex = 0;
                isBufferFull = 0;
            }
            oBufferIndex += printable;

            // How much printable bytes
            printableCount += printable;

            // How much left to read from request
            outputSize -= readNumber;

            // How much bytes read
            writtenCount += readNumber;

            // In a case outputBuffer is not empty but still not full
            // save how much left to write after the loop ends
            bufferWriteSize = min(bufferSize, oBufferIndex);
        }
    }

    // Write to output
    if (!writeBuffer(outputBuffer, ofd, bufferWriteSize, output_file)) {
		close(ofd);
    	close(ifd);
        return -1;
    }

    if(!printStatistics(charReq, writtenCount, printableCount)){
        close(ofd);
        close(ifd);
        return -1;
    }

    free(inputBuffer);
    free(outputBuffer);
    close(ofd);
    close(ifd);
    return 0;
}
