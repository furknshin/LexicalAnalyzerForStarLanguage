#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>



void dosyaya_yaz(FILE *textFile, const char *icerik) 
{
    //code.lex dosyasına yazma işlemi yapılır
    fputs(icerik, textFile);
    fputs("\n", textFile);
}

bool isKeyword(char *word) 
{
    //kelimenin keyword olup olmadığı kontrol edilir
    char *keywords[] = {"int", "text", "is", "loop", "times", "read", "write", "newLine"};
    #define keywordSize (sizeof(keywords) / sizeof(keywords[0]))

    for (int i = 0; i < keywordSize; ++i) 
    {

        if (strcmp(keywords[i], word) == 0) 
        {
            return true;
        }
    }
    
    return false;
}

bool isString(const char *word)
{
    //kelime " ile başlıyorsa true döndürülür
    if(strchr(word, '"'))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isEndOfLine(char *word)
{
    //nokta olup olmadğı kontrol edilir
    if(word[0] == '.')
    {
        return true;
    }
    return false;
}

bool isComma(char *word)
{
    //virgül olup olmadığı kontrol edilir
    if(word[0] == ',')
    {
        return true;
    }
    return false;
}

bool isOperator(char *word)
{
    //operatör olup olmadığı kontrol edilir
    if(word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/')
    {
        return true;
    }
    return false;
}

bool isInteger(char *word)
{
    //integer olup olmadığı kontrol edilir
    while (*word) {
        if (!isdigit(*word)) 
        {
            return false; 
        }
        word++;
    }
    return true;
}

bool isBracket(char *word)
{
    //bracket olup olmadığı kontrol edilir
    if(word[0]=='{' || word[0] =='}')
    {
        return true;
    }
    return false;
}


int main()
{
    FILE *sourceFile;
    FILE *textFile;
    
    // Okuma ve yazma işlemlerinin yapılacağı dosyalar açılır
    sourceFile = fopen("code.sta", "r");
    textFile = fopen("code.lex", "w");

    // isString metotu sağlandığında kullanılacak olan sabitler
    #define MAX_WORDS 1024
    #define MAX_WORD_LENGTH 1024

    // Dosyaların açılamaması koşulları
    if (sourceFile == NULL)
    {
        printf("Dosya acma hatasi!\n");
        return 1;
    }
    
    if (textFile == NULL)
    {
        printf("Dosya acma hatasi!\n");
        return 1;
    }


    char line[1024];

    while (fgets(line, sizeof(line), sourceFile) != NULL)
    {
        
        // Her satır sırayla alınır ve kopyalanır
        char copyLine[1024];
        strcpy(copyLine, line);
        
        
        int lineLength = strlen(copyLine);

        // Boşluk karakterine göre split işleminin yapılacağı satır templine'da tutulur
        char tempLine[2048] = "";
        int tempIndex = 0;


        for (int i = 0; i < lineLength; i++) 
        {
            
            // Yorum satırı kontrolü
            if(copyLine[i] == '/' && copyLine[i+1] == '*')
            {
                i++;
                // Yorum satırının kapatılmasını kontrol eder
                while(copyLine[i] != '*' || copyLine[i+1] != '/')
                {
                    if(i == lineLength)
                    {
                        printf("Lexical Analyzer Error.\nComment fails to terminate!");
                        return 1;
                    }
                    // Copyline'nin indexi arttırılarak yorum satırının içeriği templine'a aktarılmamış olur                  
                    i++;
                }
                i+=2;
            }

            // Negatif sayı atama kontrolü
            if (copyLine[i] == 'i'&& copyLine[i+1]=='s' && copyLine[i+3]=='-')
            {
                tempLine[tempIndex++] = copyLine[i];
                tempLine[tempIndex++] = copyLine[i+1];
                tempLine[tempIndex++] = copyLine[i+2];
                tempLine[tempIndex++] = '0';
                i+=4;
                while (isdigit(copyLine[i]))
                {
                    i++;
                }               
            }

            // Belirlenen karakterler geldiğinde templine'a o karakterin sağ ve sol kısımlarına boşluk konularak aktarılır
            if 
            (
                copyLine[i] == '.' || copyLine[i] == ',' || copyLine[i] == '+' || copyLine[i] == '-' || copyLine[i] == '*' || copyLine[i] == '/' || copyLine[i] == '{' || copyLine[i] == '}'
            ) 
            {
                // Karakter bulunduğunda öncesine ve sonrasına boşluk eklenir
                tempLine[tempIndex++] = ' ';
                tempLine[tempIndex++] = copyLine[i];
                tempLine[tempIndex++] = ' ';
            } 
            else 
            {
                // Diğer karakterler doğrudan aktarılır
                tempLine[tempIndex++] = copyLine[i];
            }

        }

        // Tırnak işaretinin kapatılıp kapatılmadığı kontrol edilir
        int say = 0;
        for(int s = 0; s < sizeof(tempLine); s++)
        {
            if(tempLine[s] == '"')
            {
                say++;
            }
        }
        if(say % 2 == 1)
        {
            printf("Lexical analyzer error.\nString constant cannot terminate before the file end!");
            return 1;
        }


        // Templine boşluk karakterine göre split'lenir
        char *kelime = strtok(tempLine, " \n");

        while (kelime != NULL) 
        {
            
            if (isString(kelime))
            {
                
                //Stringin tek bir kelimeden oluştuğu durum
                if(kelime[strlen(kelime)-1] == '"')
                {


                    // String'in uzunluk kontrolü yapılır
                    if(strlen(kelime) > 258)
                    {
                        printf("Lexical analyzer error.\nString constant cannot exceed 256 characters!");
                        return 1;
                    }
                    

                    char formatliString[256];
                    sprintf(formatliString, "String(%s)", kelime);       
                    dosyaya_yaz(textFile, formatliString);

                    // Sıradaki kelimeye geçilir
                    kelime = strtok(NULL, " \n");

                }
                else 
                {
                    // String'in birden fazla kelimeden oluştuğu durum

                    char words[MAX_WORDS][MAX_WORD_LENGTH];
                    int index = 0;

                    // Tırnak işaretleri arasındaki tüm kelimeler words dizisine kopyalanır
                    while(kelime[strlen(kelime)-1] != '"')
                    {
                        strcpy(words[index], kelime);
                        index++;
                        kelime = strtok(NULL, " \n");
                    }

                    strcpy(words[index], kelime);
                    
                    // Words dizisindeki her kelimeyi tek bir parça haline getirir
                    char birlesikMetin[1024] = "";
                    for (int i = 0; i < index + 1; i++) 
                    {
                        strcat(birlesikMetin, words[i]);

                        if (i < index) 
                        {
                            strcat(birlesikMetin, " "); 
                            // Son parçadan sonra boşluk eklenmez
                        }
                    }

                    // Tırnaklar da string'e dahil olduğu için string 258'den büyük olduğunda hata verilir
                    if(strlen(birlesikMetin) > 258)
                    {
                        printf("Lexical analyzer error.\nString constant cannot exceed 256 characters!");
                        return 1;
                    }

                    char formatliString[256];
                    sprintf(formatliString, "String(%s)", birlesikMetin);       
                    dosyaya_yaz(textFile, formatliString);

                    kelime = strtok(NULL, " \n");
                }

            }

            else if(isKeyword(kelime))
            {
                char formatliString[100];
                sprintf(formatliString, "Keyword(%s)", kelime);       
                dosyaya_yaz(textFile, formatliString);

                kelime = strtok(NULL, " \n");
            }

            else if (isEndOfLine(kelime))
            {
                dosyaya_yaz(textFile, "EndOfLine(.)");

                kelime = strtok(NULL, " \n");
            }
            
            else if (isComma(kelime))
            {     
                dosyaya_yaz(textFile, "Comma(,)");

                kelime = strtok(NULL, " \n");
            }
            
            else if(isOperator(kelime))
            {
                char formatliString[100];
                sprintf(formatliString, "Operator(%s)", kelime);       
                dosyaya_yaz(textFile, formatliString);

                kelime = strtok(NULL, " \n");
            }
            
            else if (isInteger(kelime))
            {

                // Sayı 8 basamaktan fazlaysa hata verilir
                if(strlen(kelime) > 8)
                {
                    printf("Lexical analyzer error.\nInteger size cannot be larger than 8 digits!");
                    return 1;
                }
                
                
                char formatliString[100];
                sprintf(formatliString, "Integer(%s)", kelime);       
                dosyaya_yaz(textFile, formatliString);

                kelime = strtok(NULL, " \n");
            }
            
            else if (isBracket(kelime))
            {
                if(kelime[0] == '{')
                {
                    dosyaya_yaz(textFile, "LeftCurlyBracket({)");
                }
                else
                {
                    dosyaya_yaz(textFile, "RightCurlyBracket(})");
                }

                kelime = strtok(NULL, " \n");
            }
            
            else
            {
                //identifier sayı ile başlarsa hata verilir
                if(isdigit(kelime[0]))
                {
                    printf("Lexical analyzer error.\nIdentifier cannot start with digit!");
                    return 1;
                }

                //identifier 10 karakterden uzunsa hata verilir
                if(strlen(kelime) > 10)
                {
                    printf("Lexical analyzer error.\nIdentifier size cannot be larger than 10 characters!");
                    return 1;
                }
                
                char formatliString[100];
                sprintf(formatliString, "Identifier(%s)", kelime);       
                dosyaya_yaz(textFile, formatliString);

                kelime = strtok(NULL, " \n");
            }
        }       
    }
    
    // dosyalar kapatılır
    fclose(sourceFile);
    fclose(textFile);
  
    return 0;
}



