#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


#define REPORT_ERR(MSG) { printf("LINE: %d, Error - %s\n", __LINE__, (MSG)); }

#define ARGC_ERR_MSG "Invalid nubmer of argument. Expected 4, but recieved"
#define FORMANT_ERR_MSG "Invalid argument foramt"
#define ALLOC_ERR_MSG "Memory allocation failed"
#define READ_FILE_ERR_MSG "Failed reading from"
#define WRITE_FILE_ERR_MSG "Failed writing to"
#define OPEN_FILE_ERR_MSG "Unable to open file"
#define CREATE_FILE_ERR_MSG "Unable to create file"

#define min(x,y) ((x) <= (y) ? (x) : (y))

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

long long getAmountKiloByte(DataAmount data);


long long getDataAmount(char *str) {
    char *tmp;
    int res = (int) strtol(str, &tmp, 10);

    DataAmount data;
    data.amount = res;
    // TODO error 
    if (0 == strcmp(tmp, "B")) {
        data.dataUnit = B;
    } else if (0 == strcmp(tmp, "K")) {
        data.dataUnit = K;
    } else if (0 == strcmp(tmp, "M")) {
        data.dataUnit = M;
    } else {
        data.dataUnit = G;
    }
    return getAmountKiloByte(data);
}

/**
 * Return if the value of the byte is between 20 and 126 
 * @param c - byte
 * @return  - 1 if the byte value is between 20 and 126,
		 	  otherwise 0.
 */
int isPrintable(char c) {
    return 32 <= c && c <= 126;
}

/**
 * Return the size of the buffer which fits to the data amount
 * requested by user. Using convension that
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
 * Print statistic in nice format
 * @param charReq 		- Number of characters requested
 * @param charRead 		- Number of characters read
 * @param charPrintable - Number of printable characters
 */
void printStatistics(long long int charReq, long long int charRead, long long int charPrintable) {
    printf("%lld characters requested, %lld characters read, %lld are printable\n", charReq, charRead, charPrintable);
}

int filterBuffer(char *inputBuffer, char *outputBuffer, int bufferSize, int readNumber, int *iBufferIndex, int oBufferIndex, char *isBufferFull) {
    if (inputBuffer == NULL || outputBuffer == NULL) {
        // TODO error message and errno
        return -1;
    }
    int i, j = 0;
    for (i = *iBufferIndex; i < readNumber; ++i) {
        if (isPrintable(inputBuffer[i])) {
            outputBuffer[oBufferIndex + j++] = inputBuffer[i];
        }
        // Write to output file If file outputBuffer 
        // is full. Set the index from which you
        // should filter start filter.
        if(oBufferIndex + j == bufferSize){
            *isBufferFull = 1;
            *iBufferIndex = ++i;
            return j;
        }
    }
    return j;
}

// TODO Write error message and errno 
char writeBuffer(char *outputBuffer, int ofd, int size) {
    ssize_t written = write(ofd, outputBuffer, (size_t) size);
    if (written < 0) {
    	// TODO error message
        return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    int bufferSize, bufferReadSize = 0, bufferWritreSize = 0;
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
    bufferSize = getBufferSize(outputSize);
    charReq = outputSize;


    inputBuffer = malloc(bufferSize * sizeof(char));
    outputBuffer = malloc(bufferSize * sizeof(char));

    if (inputBuffer == NULL || outputBuffer == NULL) {
    	printf("LINE: %d, Error - %s\n", __LINE__, ALLOC_ERR_MSG);
        goto freeMem;
    }

    ifd = open(input_file, O_RDONLY, S_IRUSR);
    if(ifd == -1){
    	printf("LINE: %d, Error - %s %s\n", __LINE__, OPEN_FILE_ERR_MSG, input_file);
    	goto freeMem;
    }

    ofd = creat(output_file, S_IRUSR);
    if(ofd == -1){
    	printf("LINE: %d, Error - %s %s\n", __LINE__, CREATE_FILE_ERR_MSG, input_file);
    	goto freeMem;
    }

    ofd = open(output_file, O_WRONLY, S_IWUSR);
    if(ofd == -1){
    	printf("LINE: %d, Error - %s %s\n", __LINE__, OPEN_FILE_ERR_MSG, input_file);
    	
    }
    if (ifd == -1 || ofd == -1) {
        goto freeMem;
    }


    while (outputSize > 0) {
        bufferReadSize = min(outputSize, bufferSize);
        readNumber = read(ifd, inputBuffer, (size_t) bufferReadSize);

	    if (readNumber < 0) {
	    	printf("LINE: %d, Error - %s %s\n", __LINE__, READ_FILE_ERR_MSG, input_file);
	        goto freeMem;
	    }
        if(readNumber == 0){ 
            // EOF, Reset pointer if we read all the file 
            lseek(ifd, 0, SEEK_SET);            
        } else {
            printf("%zu %d\n", readNumber, bufferReadSize);
            printable = filterBuffer(inputBuffer, outputBuffer, bufferSize,readNumber, &iBufferIndex, oBufferIndex, &isBufferFull);
            if (printable == -1){
                // TODO error
                goto freeMem;
            }
            
            if(isBufferFull){
                // bufferWritreSize = min(outputSize, bufferSize);
                oBufferIndex = 0;
                readNumber -= iBufferIndex;
                printableCount += printable;        // How much printable bytes
                // Write to output
                if(!writeBuffer(outputBuffer, ofd, bufferSize)){
                    // TODO error
                    goto freeMem;
                }

                // Fill the output buffer with bytes left in input buffer
                oBufferIndex = 0;
                printable = filterBuffer(inputBuffer, outputBuffer, bufferSize,readNumber, &iBufferIndex, oBufferIndex, &isBufferFull);
                printableCount += printable;    // How much printable bytes
                iBufferIndex = 0;
                isBufferFull = 0;
            } else {
                oBufferIndex += printable;
                printableCount += printable;    // How much printable bytes
            }

            outputSize -= readNumber;           // How much left to read
                                                // from request
            writtenCount += readNumber;         // How much bytes read
        }
    }

	printStatistics(charReq, writtenCount, printableCount);
	
    freeMem:
    close(ofd);
    close(ifd);
    free(inputBuffer);
    free(outputBuffer);


    return 0;
}
