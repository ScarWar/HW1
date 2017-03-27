#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


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

long getAmountKiloByte(DataAmount data);

/**
 *
 * @param str
 * @return
 */
long getDataAmount(char *str) {
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
    return 20 <= c && c <= 126;
}

/**
 * Return the size of the buffer which fits to the data amount
 * requested by user. Using convension that
 * - 1028B = K
 * - 1028K = M
 * - 1028M = G
 * data amount received by user
 * @param data - Amount of data to read
 * @return Buffer size
 */
int getBufferSize(DataAmount data) {
    long int amount = getAmountKiloByte(data);
    if (data.amount >= 1073741824) { // 1024 ^ 3 = 1048576
        return 4096;
    } else if (data.amount >= 1048576) { // 1024 ^ 2 = 1048576
        return 2048;
    } else if (data.amount >= 1024) {
        return 1024;
    }
    return 512;
}


/**
 * Return amount of data in kilobyte
 * @param data - Data anount 
 * @return amount of data in kilobyte
 */
long getAmountKiloByte(DataAmount data) {
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
void printStatistcs(int charReq, int charRead, int charPrintable) {
    printf("%d characters requested, %d characters read, %d are printable\n", charReq, charRead, charPrintable);
}

int main(int argc, char **argv) {
    size_t bufferSize;
    char *buffer;
    long int dataSize;

    // Data amount, input, output
    char *data, *input, *output;

    if (argc != 4) {
        // TODO error message number of argument is invalid
        return 0;
    }

    data = argv[1];   // Data amount
    input = argv[2];  // Input file name
    output = argv[3]; // Output file name

    dataSize = getDataAmount(data);
    bufferSize = getBufferSize(dataSize);
    long int inputSize = dataSize;

    buffer = malloc(bufferSize * sizeof(char)); // TODO check alloc

    int ifd, ofd;

    if ((ifd = open(input, O_RDONLY, S_IRUSR)) < 0) {
        // TODO error message
        goto freeMem;
        return 0;
    }
    if (0 > (ifd = open(output, O_WRONLY, S_IWUSR))) {
        // TODO error message
        return 0;
    }


    ssize_t readNumber;
    ssize_t written = 0;
    readNumber = read(ifd, buffer, bufferSize);
    if (readNumber < 0) {
        // TODO error message
        return 0;
    }

    if (readNumber == 0) {
        // file is empty
        return 0;
    }

    while (inputSize > 0) {
        if (0 > (readNumber = read(ifd, buffer, bufferSize))) {
            // TODO error
        }
        inputSize -= readNumber;
        written = 0;
        while (readNumber > 0) {
            written = write(ofd, buffer, readNumber);
            if (written < 0) {
                // TODO error message
            }
            readNumber -= written;
        }
    }

    freeMem:
    free(buffer);
    return 0;
}