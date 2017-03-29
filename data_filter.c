#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <time.h>


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


/**
 *
 * @param str
 * @return
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
int getBufferSize(long long amount) {
    if (amount >= 1073741824) { // 1024 ^ 3 = 1048576
        return 4096;
    } else if (amount >= 1048576) { // 1024 ^ 2 = 1048576
        return 2048;
    } else if (amount >= 1024) {
        return 1024;
    }
    return 512;
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
void printStatistics(int charReq, int charRead, int charPrintable) {
    printf("%d characters requested, %d characters read, %d are printable\n", charReq, charRead, charPrintable);
}

int filterBuffer(char *inputBuffer, char *outputBuffer, int bufferSize) {
    if (inputBuffer == NULL || outputBuffer == NULL) {
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

char writeBuffer(const char *inputBuffer, int ofd, int size) {
    while (size > 0) {
        int written = (int) write(ofd, inputBuffer, (size_t) size);
        if (written < 0) {
            return 0;
        }
        size -= written;
    }
    return 1;
}

int main(int argc, char **argv) {
    int bufferSize;
    char *inputBuffer = NULL, *outputBuffer = NULL;
    long long inputSize;

    // Data amount, input, output
    char *data, *input_file, *output_file;

    // Input file descriptor, output file descriptor
    int ifd = 0, ofd = 0;


    ssize_t readNumber;
    int printable = 0;
    ssize_t writtenCount = 0, printableCount = 0;
    long long int fileLength, fileLengthCounter;


    if (argc != 4) {
        // TODO error message number of argument is invalid
        return 0;
    }

    data = argv[1];         // Data amount
    input_file = argv[2];   // Input file name
    output_file = argv[3];  // Output file name

    inputSize = getDataAmount(data);
    bufferSize = 50;//getBufferSize(inputSize);

    inputBuffer = malloc(bufferSize * sizeof(char));
    outputBuffer = malloc(bufferSize * sizeof(char));

    if (inputBuffer == NULL || outputBuffer == NULL) {
        // TODO malloc error message
        goto freeMem;
    }

    ifd = open(input_file, O_RDONLY, S_IRUSR);
    ofd = open(output_file, O_WRONLY, S_IWUSR);
    if (ifd == -1 || ofd == -1) {
        // TODO error message
        printf("not so lol\n");
        goto freeMem;
    }

//    printf("ifd: %d, ofd: %d\n", ifd, ofd);
    fileLength = lseek(ifd, 0, SEEK_END);
    lseek(ifd, 0, SEEK_SET);
    fileLengthCounter = fileLength;

    readNumber = read(ifd, inputBuffer, (size_t) bufferSize);
    if (readNumber < 0) {
        // TODO error message
        printf("not so lol 3\n");
        return 0;
    }

    if (readNumber == 0) {
        // TODO  file is empty
        return 0;
    }
    inputSize -= readNumber;
    fileLengthCounter -= readNumber;

    printable = filterBuffer(inputBuffer, outputBuffer, bufferSize);

    if (printable == -1 || !writeBuffer(outputBuffer, ofd, printable)) {
        goto freeMem;
    }
    printableCount += printable;
    writtenCount += printable;

//    sleep(1);
    while (inputSize > 0) {
        readNumber = read(ifd, inputBuffer, (size_t) bufferSize);
        if (readNumber < 0) {
            // TODO error
            goto freeMem;
        }
        inputSize -= readNumber;
        fileLengthCounter -= readNumber;

        printable = filterBuffer(inputBuffer, outputBuffer, bufferSize);

        if (printable == -1 || !writeBuffer(inputBuffer, ofd, printable)) {
            goto freeMem;
        }
        writtenCount += printable;
        printableCount += printable;

        if (readNumber == 0 && fileLengthCounter == 0) {
            lseek(ifd, 0, SEEK_SET);
            fileLengthCounter = fileLength;
        }
    }

    freeMem:
    close(ofd);
    close(ifd);
    free(inputBuffer);
    free(outputBuffer);


    return 0;
}
