#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


#define REPORT_ERR(MSG) { printf("LINE: %d, Error - %s", __LINE__, (MSG)); }

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

int filterBuffer(char *inputBuffer, char *outputBuffer, int bufferSize) {
    if (inputBuffer == NULL || outputBuffer == NULL) {
        // TODO error message and errno
        return -1;
    }
    int i, j = 0;
    for (i = 0; i < bufferSize; ++i) {
        if (isPrintable(inputBuffer[i])) {
            outputBuffer[j++] = inputBuffer[i];
        }
    }
    return j;
}

// TODO Write error message and errno 
char writeBuffer(const char *outputBuffer, int ofd, int size) {
    // while (size > 0) {
        ssize_t written = write(ofd, outputBuffer, (size_t) size);
        if (written < 0) {
            printf("%zu\n", written);
        	// TODO error message
            return 0;
        }
        // size -= written;
    // }
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

    char isEmpty;
    int printable = 0;
    ssize_t readNumber, writtenCount = 0, printableCount = 0;
    long long int fileLength, fileLengthCounter;
    struct stat st;


    if (argc != 4) {
    	printf("LINE: %d, Error - %s %d", __LINE__, ARGC_ERR_MSG, argc);
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
    	printf("LINE: %d, Error - %s", __LINE__, ALLOC_ERR_MSG);
        goto freeMem;
    }

    ifd = open(input_file, O_RDONLY, S_IRUSR);
    if(ifd == -1){
    	printf("LINE: %d, Error - %s %s", __LINE__, OPEN_FILE_ERR_MSG, input_file);
    	goto freeMem;
    }

    ofd = creat(output_file, S_IRUSR);
    if(ofd == -1){
    	printf("LINE: %d, Error - %s %s", __LINE__, CREATE_FILE_ERR_MSG, input_file);
    	goto freeMem;
    }

    ofd = open(output_file, O_WRONLY, S_IWUSR);
    if(ofd == -1){
    	printf("LINE: %d, Error - %s %s", __LINE__, OPEN_FILE_ERR_MSG, input_file);
    	
    }
    if (ifd == -1 || ofd == -1) {
        goto freeMem;
    }

    // fileLength = lseek(ifd, 0, SEEK_END);
    // lseek(ifd, 0, SEEK_SET);

    stat(input_file, &st);
    fileLength = st.st_size;
    fileLengthCounter = fileLength;

    printf("%jd\n", st.st_size);
    if(!fileLength){
    	isEmpty = 1;
    }


    while (outputSize > 0) {
    	bufferReadSize = min(bufferSize, outputSize);
    	if(!isEmpty){
    		bufferReadSize = min(bufferReadSize, fileLengthCounter);
    	}
        readNumber = read(ifd, inputBuffer, (size_t) bufferReadSize);
        if (readNumber != bufferReadSize){
        	// TODO error message
        	goto freeMem;
        }
	    if (readNumber < 0) {
	    	printf("LINE: %d, Error - %s %s", __LINE__, READ_FILE_ERR_MSG, input_file);
	        goto freeMem;
	    }
        printable = filterBuffer(inputBuffer, outputBuffer, readNumber);

        if (printable == -1 || !writeBuffer(outputBuffer, ofd, printable)) {
            goto freeMem;
        }
        printf("%lld\n", outputSize);
        outputSize -= readNumber; 		 	// How much left to read
        									// from request
        fileLengthCounter -= readNumber; 	// How much left to 
        								 	// read from input file
        writtenCount += readNumber;			// How much bytes read
        printableCount += printable;        // How much printable bytes

        // Reset pointer if we read all the file 
        if (fileLengthCounter == 0) {
            lseek(ifd, 0, SEEK_SET);
            fileLengthCounter = fileLength;
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
