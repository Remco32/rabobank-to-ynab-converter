#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXSTRING 30 //limit for a single slot is 30
#define AMOUNTOFSLOTS 19 //rabo uses 19 slots
#define MAXFULLSTRING 600 //limit for the whole string is MAXSTRING*AMOUNTOFSLOTS

char selectedAccount[MAXSTRING];


/*	Remco Pronk, 26-07-16 
	Converts the bankstatement format of the Rabobank to a csv-file that is readable by YNAB */

/* inputfile format

12 columns of importance
account# (IBAN) || Currency || Date (yyyymmdd) || Debet/Credit || value (3.00) || receiver account# (IBAN) ||
Receiver (Name) || Date || type (ba, bg, etc) || ??? (empty) || description || comment

Example 1:
"NL12RABO3456789012","EUR","20140318","D","3.00","","","20140318","ba","","Rest Bernoulliborg GRONINGEN","Betaalautomaat 14:42 pasnr. 004","","","","","","",""

Example 2:
"NL12RABO3456789012","EUR","20140320","C","8.84","NL98RABO7654321012","HEKSTRA J A","20140320","cb","","examenbundel","","","","","","SCT2014031937154000000002001","",""

*/

/* outputDatumfile format:

6 colums, with first line being how the csv-file is formatted

Date,Payee,Category,Memo,Outflow,Inflow
date (dd/mm/yyyy) || from/to || category (YNAB) || comment || outflow (100.00) || inflow (50.00)

Example 1 result:
"NL12RABO3456789012","EUR","20140318","D","3.00","","","20140318","ba","","Rest Bernoulliborg GRONINGEN","Betaalautomaat 14:42 pasnr. 004","","","","","","",""
18/03/2014, Rest Bernoulliborg GRONINGEN, , Betaalautomaat 14:42 pasnr. 004, 3.00, ,

Example 2 result:
"NL12RABO3456789012","EUR","20140320","C","8.84","NL98RABO7654321012","HEKSTRA J A","20140320","cb","","examenbundel","","","","","","SCT2014031937154000000002001","",""
20/03/2014, HEKSTRA J A, , examenbundel, , 8.84

*/

void reformatDate(char *date) {
    char reformattedDate[11];

    char year[5];
    char month[3];
    char day[3];
    int i;

    for (i = 0; i < 4; i++) {
        year[i] = date[i];
        reformattedDate[i + 6] = year[i];
    }
    year[4] = '\0';
    for (i = 0; i < 2; i++) {
        month[i] = date[i + 4];
        reformattedDate[i + 3] = month[i];
    }
    month[2] = '\0';
    for (i = 0; i < 2; i++) {
        day[i] = date[i + 6];
        reformattedDate[i] = day[i];
    }
    day[2] = '\0';
    reformattedDate[2] = '/';
    reformattedDate[5] = '/';
    reformattedDate[10] = '\0';

    strncpy(date, reformattedDate, 11);
}

void reformatAndPrintString(char *input, FILE *outputFilePointer) {
    int i; // iterator variable
    char seperatedInput[AMOUNTOFSLOTS][MAXSTRING]; // location to save the seperated data (date, amount, etc) from the input.

    //empty array at start
    memset(seperatedInput, 0, sizeof seperatedInput);

    //skip the first quotationmark
    input++;

    //read input to the array
    for (i = 0; i < AMOUNTOFSLOTS; i++) {
        sscanf(input, "%[^'\"']", seperatedInput[i]);
        input = strchr(input, '\"');
        //Skip the next quotationmark, comma and quotationmark by moving the pointer forwards
        input += 3;
    }

    //reformat date
    reformatDate(seperatedInput[2]);

    //if there is no receiver , then it is a pin transaction: receiver becomes memo field, 2nd memo becomes first memo
    if (seperatedInput[6][0] == NULL) {
        printf("There was no receiver field: this is a pin transaction.\n");

        strncpy(seperatedInput[6], seperatedInput[10], MAXSTRING);

        //empty memo1 - only memo2 gets printed this way
        memset(seperatedInput[10], 0, MAXSTRING);


        //printf("Strings are now %s and %s\n", seperatedInput[6], seperatedInput[10]);
    }

    if (strcmp(selectedAccount, seperatedInput[0]) == 0) { //exact match with input
        //print to file
        //Output order is different for credit and debet
        if (seperatedInput[3][0] == 'C') {
            printf("(Date) %s || (Name SENDER) %s || (Category) || (Comment) %s%s || (Outflow) || (Inflow) %s\n\n",
                   seperatedInput[2], seperatedInput[6], seperatedInput[10], seperatedInput[11], seperatedInput[4]);
            //We don't fill in the category slot
            fprintf(outputFilePointer, "%s,%s,,%s%s,,%s\n", seperatedInput[2], seperatedInput[6], seperatedInput[10],
                    seperatedInput[11], seperatedInput[4]);
        }
        if (seperatedInput[3][0] == 'D') {
            printf("(Date) %s || (Name RECEIVER) %s || (Category) || (Comment) %s%s || (Outflow) %s || (Inflow) \n\n",
                   seperatedInput[2], seperatedInput[6], seperatedInput[10], seperatedInput[11], seperatedInput[4]);
            fprintf(outputFilePointer, "%s,%s,,%s%s,%s,\n", seperatedInput[2], seperatedInput[6], seperatedInput[10],
                    seperatedInput[11], seperatedInput[4]);
        }
    }
}

void readInput(FILE *ifp, FILE *ofp) {
    char inputLine[MAXFULLSTRING];

    //Print first line
    fprintf(ofp, "Date,Payee,Category,Memo,Outflow,Inflow\n");
    do {
        fscanf(ifp, "%[^\n]", inputLine);
        //Rabobank transaction files end with an empty line, we catch that here
        if (strlen(inputLine) == 1) {
            return;
        }
        //reformat the the line we just read
        reformatAndPrintString(inputLine, ofp);
    } while (fgets(inputLine, MAXFULLSTRING, ifp) != NULL); //scan next line

    //Done with the file, close it.
    fclose(ifp);
}

void stopProgramAfterInput() {
    printf("\nThe application has finished. Press any key to continue.");
    getchar(); //to not close application without user interaction
    exit(0);
}

void readSettings() {
    //TODO make universal by making user provide the slot of each piece of information, and adding flags for certain cleanups (i.e remove quotationmarks, commas, etc)
    FILE *fpSettings = fopen("settingsYNABConverter.ini", "r+"); //Allow reading AND writing
    if (fpSettings) {
        char input[MAXFULLSTRING];
        printf("Setting file exists.\n");

        //read first line
        fscanf(fpSettings, "%[^\n]", input);
        //copy whatever is after the '=' to the used account number
        strncpy(selectedAccount, strchr(input, '=') + 1, MAXSTRING);


    }
    else { //No file exists
        printf("No setting file exists, creating one...\n");
        //create new ini file
        FILE *opSettings = fopen("settingsYNABConverter.ini", "w");

        //populate file
        fprintf(opSettings, "AccountnumberToUse=NL12RABO3456789012");
        printf("Setting file created in the directory of the program named 'settingsYNABConverter.ini'. Manually edit the file to add an account number to use with this program.\n");
        fclose(opSettings);
        stopProgramAfterInput();
    }
}

//TODO clean up main: make functions
int main(int argc, char *argv[]) {
    //reserve memory for input and output
    FILE *ifp;
    FILE *ofp;

    readSettings();

    //In case of no inputfile given, the program can not work.
    if (argv[1] == NULL) {
        printf("There was no inputfile.\nDrag and drop a file unto the executable to process it.\n");
        stopProgramAfterInput();
    }



    //Open the inputfile and save it to inputFilePointer
    ifp = fopen(argv[1], "r");
    //Create a file to write to: outputFilePointer
    ofp = fopen("output.csv", "w");
    //read, parse and reformat the input
    readInput(ifp, ofp);
    //close the output file
    fclose(ofp);


    //end the application
    exit(0);
}
