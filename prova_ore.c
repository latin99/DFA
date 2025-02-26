#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* DEFINES */
#define           CC_4_5    "scarica CC 4.5A"
#define            CC_15    "scarica CC 15A"
#define            CC_30    "scarica CC 30A"
#define            CC_45    "scarica CC 45A"
#define           CW_110    "scarica CW 110W"
#define           CW_112    "scarica CW 112W"
#define       MAX_POINTS    10000
#define          MAX_STR    500
#define           MAX_CH    8 
#define          VERSION    "V_1.0"
#define    DIFFERENZIALE    0.000277778

/* STRUCTURES */
typedef struct data{
    int ore;
    int minuti;
    int secondi;
} data;

typedef struct tempi_prova{
    struct data start_prova;
    struct data end_prova;
    struct data start_scarica;
    struct data end_scarica;
    struct data durata_prova;
    struct data durata_scarica;
    float durata_scarica_sec;
} tempi_prova;

typedef struct misure_prova{
    float corrente[MAX_POINTS];
    int punti_parziali;
    float corrente_scarica_batt[MAX_POINTS];
    int punti_corrente_scarica_batt;
    float corrente_scarica_batt_offset_p[MAX_POINTS];
    int punti_corrente_scarica_batt_offset_p;
    float corrente_scarica_batt_offset_n[MAX_POINTS];
    int punti_corrente_scarica_batt_offset_n;
    float tensione_batt_vuoto[MAX_POINTS];
    int punti_tensione_batt_vuoto;
    float tensione_scarica_batt[MAX_POINTS];
    int punti_tensione_scrica_batt;
    //float capacita_calcolata;
} misure_prova;

typedef struct etichette{
    char nomi[MAX_CH][MAX_STR];
    int channel_attivi;
} etichette;

typedef struct prova{
    char tipo_prova[MAX_STR];
    int fattore_pinza;
    int fattore_tensione;
    int punti_totali;
    FILE *fp_acquisizioni;
    FILE *fp_risultato;
    struct misure_prova risultati_prova;
    struct tempi_prova tempi;
    struct etichette labels;
} dati_prove;

/* FUNCTIONS */
void inizializza_struttura (dati_prove* dp);
void uso_eseguibile (void);
int nome_scarica_corretta (char argv[]);
int fattore_pinza (char pinza[]);
void sistema_file (char file[]);
void red (void);
void green (void);
void reset_color (void);
void elabora_file_dati (dati_prove* dp, char nome_file[]);
void estrai_punti_tot (dati_prove* dp, char r[]);
void estrai_inizio_prova (dati_prove* dp, char r[]);
void estrai_fine_prova (dati_prove* dp, char r[]);
void estrai_etichette (dati_prove* dp, char r[], int channel);
void estrai_misure (dati_prove* dp, char r[]);
void shift (char s[]);
float media (float v[], int dim);
void calcoli (dati_prove* dp);
//void calcoli_temporali(tempi_prova* t);
//tempi_prova sottrai_date (data d1, data d2);
int max (int n1, int n2);
void creazione_pagina_web (float f1, float f2, float f3, float f4, float f5, int i1);

/* per eseguire l'eseguibile dovro' lanciare il comando "./<nome_eseguibile> <TIPO_DI_SCARICA>"
   TIPO_DI_SCARICA:
       - CC_4_5  | cc_4_5
       - CC_15   | cc_15
       - CC_30   | cc_30
       - CC_45   | cc_45
       - CC_110  | cw_110
       - CW_112  | cw_112
*/
int main (int argc, char **argv){
    dati_prove dp;

    inizializza_struttura(&dp);
    
    //controlli sull'eseguibile e i parametri
    if((argc != 4) || !nome_scarica_corretta(argv[1]) || !fattore_pinza(argv[2])){
        uso_eseguibile();
        exit(EXIT_FAILURE);
    }else{
        green(); printf("Discharge File Analyzer - DFA version %s\n", VERSION); reset_color(); fflush(stdout);
        sistema_file(argv[3]);
        strcpy(dp.tipo_prova, argv[1]);
        dp.fattore_pinza = atoi(argv[2]);
    }

    elabora_file_dati(&dp, argv[3]);
    
    //calcolo capacità finale --> prova
    calcoli(&dp);
    
    return EXIT_SUCCESS;
}

void inizializza_struttura (dati_prove* dp){
    dp->labels.channel_attivi = 0;
    //dp->fattore_tensione = 1000;
    dp->risultati_prova.punti_corrente_scarica_batt = 0;
    dp->risultati_prova.punti_parziali = 0;
    dp->risultati_prova.punti_corrente_scarica_batt_offset_n = 0;
    dp->risultati_prova.punti_corrente_scarica_batt_offset_p = 0;
    dp->risultati_prova.punti_tensione_batt_vuoto = 0;
    dp->risultati_prova.punti_tensione_scrica_batt = 0;
    dp->punti_totali = 0;
    
    for(int i = 0; i < MAX_POINTS; i++){
        dp->risultati_prova.corrente[i] = 0;
        dp->risultati_prova.corrente_scarica_batt[i] = 0;
        dp->risultati_prova.corrente_scarica_batt_offset_n[i] = 0;
        dp->risultati_prova.corrente_scarica_batt_offset_p[i] = 0;
        dp->risultati_prova.tensione_batt_vuoto[i] = 0;
        dp->risultati_prova.tensione_scarica_batt[i] = 0;       
    }
}

void uso_eseguibile (void){
    red(); printf("Ricontrolla i dati inseriti!\nUso dell'eseguibile:\n"); fflush(stdout);
        printf("    - ./<nome_eseguibile> <TIPO_DI_SCARICA> <FATTORE_DI_CONVERSIONE_PINZA> <nome_file.csv>\n"); fflush(stdout);
        printf("    - TIPO_DI_SCARICA:\n");
        printf("        - CC_4_5  | cc_4_5\n"); fflush(stdout);
        printf("        - CC_15   | cc_15\n"); fflush(stdout);
        printf("        - CC_30   | cc_30\n"); fflush(stdout);
        printf("        - CC_45   | cc_45\n"); fflush(stdout);
        printf("        - CC_110  | cw_110\n"); fflush(stdout);
        printf("        - CW_112  | cw_112\n"); fflush(stdout);
        printf("    - FATTORE_DI_CONVERSIONE_PINZA:\n"); fflush(stdout);
        printf("        - 10  --> per correnti fino a 100A\n"); fflush(stdout);
        printf("        - 100 --> per correnti fino a 10A\n"); fflush(stdout); reset_color();
}

int nome_scarica_corretta (char argv[]){
    if(!strcmp(argv, "CC_4_5") || !strcmp(argv, "cc_4_5") || !strcmp(argv, "CC_15") || !strcmp(argv, "cc_15") || !strcmp(argv, "CC_30") || !strcmp(argv, "cc_30") || !strcmp(argv, "CC_45") || !strcmp(argv, "cc_45") || !strcmp(argv, "CW_110") || !strcmp(argv, "cw_110") ||!strcmp(argv, "CW_112") ||!strcmp(argv, "cw_112"))
        return 1;
    else
        return 0;
}

int fattore_pinza (char pinza[]){
    if(!strcmp("10", pinza) || !strcmp("100", pinza))
        return 1;
    else
        return 0;
}

void sistema_file (char file[]){
    char command[MAX_STR];
    
    sprintf(command, "cat %s | tr ',' ' ' > file_lavoro.csv", file);
    system(command);
    strcpy(file, "file_lavoro.csv");
}

void red (void){
    printf("\033[31;1m"); fflush(stdout);
}

void green (void){
    printf("\033[32;1m"); fflush(stdout);
}

void reset_color (void){
    printf("\033[0m"); fflush(stdout);
}

void elabora_file_dati (dati_prove* dp, char nome_file[]){
    char riga[MAX_STR];
    int i = 0;

    if((dp->fp_acquisizioni = fopen(nome_file, "r+")) != NULL){
        green(); printf("\nFile aperto correttamente in lettura e scrittura\n"); fflush(stdout); reset_color();
    }else{
        red(); printf("File non aperto!\n"); fflush(stdout); reset_color();
    }

    while(fgets(riga, MAX_STR, dp->fp_acquisizioni) != NULL){
        if(i == 6){    //le prime 6 righe le butto via, non hanno info utili
            estrai_punti_tot(dp, riga);
        }else if(i == 8){
            estrai_inizio_prova(dp, riga);
        }else if(i == 9){
            estrai_fine_prova(dp, riga);
        }else if(i >= 13 && i <= 20){
            estrai_etichette(dp, riga, i);
        }else if(i >= 24){
            estrai_misure(dp, riga);
        }
        i++;
    }
    
    fclose(dp->fp_acquisizioni);
}

void estrai_punti_tot (dati_prove* dp, char r[]){
    int i = 0, j = 0;
    char punti_totali[10];

    //printf("%s\n", r); fflush(stdout);
    for(i = 0; i < strlen(r); i++){
        while(isdigit(r[i])){
            punti_totali[j] = r[i];
            i++; j++;
        }
    }
    punti_totali[j] = '\n';
    dp->punti_totali = atoi(punti_totali);
    //printf(">>%d<<\n", dp->punti_totali); fflush(stdout);

}

void estrai_inizio_prova (dati_prove* dp, char r[]){
    char start[10], ora[3], min[3], sec[3];

    //printf("%s\n", r); fflush(stdout);
    sscanf(r, "%*s %*s %*s %s", start);
    
    ora[0] = start[0]; ora[1] = start[1]; ora[3] = '\n';
    dp->tempi.start_prova.ore = atoi(ora);

    min[0] = start[3]; min[1] = start[4]; min[3] = '\n';
    dp->tempi.start_prova.minuti = atoi(min);

    sec[0] = start[6]; sec[1] = start[7]; sec[3] = '\n';
    dp->tempi.start_prova.secondi = atoi(sec);

    //printf(">>%d<< >>%d<< >>%d<<\n", dp->tempi.start_prova.ore, dp->tempi.start_prova.minuti, dp->tempi.start_prova.secondi); fflush(stdout);
}

void estrai_fine_prova (dati_prove* dp, char r[]){
    char end[10], ora[3], min[3], sec[3];

    //printf("%s\n", r); fflush(stdout);
    sscanf(r, "%*s %*s %*s %s", end);
    
    ora[0] = end[0]; ora[1] = end[1]; ora[3] = '\n';
    dp->tempi.end_prova.ore = atoi(end);

    min[0] = end[3]; min[1] = end[4]; min[3] = '\n';
    dp->tempi.end_prova.minuti = atoi(min);

    sec[0] = end[6]; sec[1] = end[7]; sec[3] = '\n';
    dp->tempi.end_prova.secondi = atoi(sec);

    //printf(">>%d<< >>%d<< >>%d<<\n", dp->tempi.end_prova.ore, dp->tempi.end_prova.minuti, dp->tempi.end_prova.secondi); fflush(stdout);
}

void estrai_etichette (dati_prove* dp, char r[], int channel){
    char ch_active[4], label[MAX_STR];
    
    sscanf(r, "%s %s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s", ch_active, label);
    
    //printf(">>%s | %s | %ld<<\n", ch_active, label, strlen(ch_active)); fflush(stdout);

    if(strlen(ch_active) == 3){
        strcpy(dp->labels.nomi[channel], label);
        dp->labels.channel_attivi++;
    }else{
        return;
    }
    //printf(">>%s<<\n", dp->labels.nomi[channel]); fflush(stdout);
}

void estrai_misure (dati_prove* dp, char r[]){
    int indice;
    float tensione, corrente;
    char tempo[10], temp[dp->labels.channel_attivi][MAX_STR], t[8], c[8];
    
    sscanf(r, "%d %*s %s %*s %s %s %s %s %s %s %s %*s %*s",
               &indice, tempo, t, c, temp[0], temp[1], temp[2], temp[3], temp[4]);
    
    //red();printf("%d | %s | %s | %s\n", indice, tempo, t, c); fflush(stdout);reset_color();
    //printf("%d | %f | %f\n", indice, tensione, corrente); fflush(stdout);
    
    shift(t); shift(c);
    //printf("%d | %s | %s\n",indice, t, c); fflush(stdout);
    tensione = atof(t);
    corrente = atof(c) * dp->fattore_pinza;
    //printf("%d | %f | %f\n",indice, tensione, corrente); fflush(stdout);
    
    indice--;
    dp->risultati_prova.corrente[indice] = corrente;
    dp->risultati_prova.punti_parziali++;

    if(dp->risultati_prova.corrente[indice] <= 1 && dp->risultati_prova.corrente[indice] >= 0){       
        dp->risultati_prova.corrente_scarica_batt_offset_p[dp->risultati_prova.punti_corrente_scarica_batt_offset_p] = corrente;
        dp->risultati_prova.punti_corrente_scarica_batt_offset_p++;    
        
        if(media(dp->risultati_prova.corrente, dp->risultati_prova.punti_parziali) < 1){
            dp->risultati_prova.tensione_batt_vuoto[dp->risultati_prova.punti_tensione_batt_vuoto] = tensione;
            dp->risultati_prova.punti_tensione_batt_vuoto++;
        }
    }else if(dp->risultati_prova.corrente[indice] < 0){
        dp->risultati_prova.corrente_scarica_batt_offset_n[dp->risultati_prova.punti_corrente_scarica_batt_offset_n] = corrente;
        dp->risultati_prova.punti_corrente_scarica_batt_offset_n++;
        
        if(media(dp->risultati_prova.corrente, dp->risultati_prova.punti_parziali) < 1){
            dp->risultati_prova.tensione_batt_vuoto[dp->risultati_prova.punti_tensione_batt_vuoto] = tensione;
            dp->risultati_prova.punti_tensione_batt_vuoto++;
        }
    }else{
        dp->risultati_prova.corrente_scarica_batt[dp->risultati_prova.punti_corrente_scarica_batt] = corrente;
        dp->risultati_prova.punti_corrente_scarica_batt++;
        dp->risultati_prova.tensione_scarica_batt[dp->risultati_prova.punti_tensione_scrica_batt] = tensione;
        dp->risultati_prova.punti_tensione_scrica_batt++;
    }
}

void shift (char s[]){
    int len = strlen(s);

    if(len > 0){
        for(int i = 0; i < len; i++){
            s[i] = s[i + 1];
        }
        s[len - 1] = '\0';
    }else{
        return;
    }
}

float media (float v[], int dim){
    int i;
    float s = 0;

    for(i = 0; i < dim; i++){
        s = s + v[i];
    }
    return s / dim;
}

void calcoli (dati_prove* dp){
    float capacita_finale = 0;
    float capacita_finale_offset = 0;
    float tensione_media_vuoto;
    float offset_n = 0;
    float offset_p = 0;
    float offset_tot_medio = 0;
    float corrente_scarica_media = 0;
    float corrente_scarica_media_offset = 0;
    int i;
    char pagina_web[5000], pagina_web1[100], pagina_web2[100], pagina_web3[100];
    
    //calcoli_temporali(dp->tempi);
    
    // capacita' finale senza offset con "integrale"-----------------------------------------------------------
    for(i = 0; i < dp->risultati_prova.punti_corrente_scarica_batt; i++){
        //printf("%d: %f\n", i, dp->risultati_prova.corrente_scarica_batt[i]); fflush(stdout);
        capacita_finale += (dp->risultati_prova.corrente_scarica_batt[i] * DIFFERENZIALE);
        corrente_scarica_media += dp->risultati_prova.corrente_scarica_batt[i];
    }
    
    // tensione a vuoto media----------------------------------------------------------------------------------
    tensione_media_vuoto = media(dp->risultati_prova.tensione_batt_vuoto, dp->risultati_prova.punti_tensione_batt_vuoto);
    
    // offset negativo-----------------------------------------------------------------------------------------
    for(i = 0; i < dp->risultati_prova.punti_corrente_scarica_batt_offset_n; i++){
        offset_n += dp->risultati_prova.corrente_scarica_batt_offset_n[i];
    }

    // offset positivo-----------------------------------------------------------------------------------------
    for(i = 0; i < dp->risultati_prova.punti_corrente_scarica_batt_offset_p; i++){
        //printf("%d: %f\n", i, dp->risultati_prova.corrente_scarica_batt_offset_p[i]); fflush(stdout);
        offset_p += dp->risultati_prova.corrente_scarica_batt_offset_p[i];
    }

    // offset totale medio (negativo medio - positivo medio)---------------------------------------------------
    if(offset_n == 0)         offset_tot_medio = - offset_p / dp->risultati_prova.punti_corrente_scarica_batt_offset_p;
    else if(offset_p == 0)    offset_tot_medio =   offset_n / dp->risultati_prova.punti_corrente_scarica_batt_offset_n; 
    else                      offset_tot_medio = (offset_n / dp->risultati_prova.punti_corrente_scarica_batt_offset_n) - (offset_p / dp->risultati_prova.punti_corrente_scarica_batt_offset_p);

    // corrente scarica media con offset-----------------------------------------------------------------------
    corrente_scarica_media_offset = (corrente_scarica_media / dp->risultati_prova.punti_corrente_scarica_batt) + offset_tot_medio;
    // TODO!!
    //per porter utilizzare la corrente_scarica_media_offset mi servono i tempi!!

    // capacita' finale con offset totale MEDIO----------------------------------------------------------------
    for(i = 0; i < dp->risultati_prova.punti_corrente_scarica_batt; i++){
        capacita_finale_offset += ((dp->risultati_prova.corrente_scarica_batt[i] + offset_tot_medio) * DIFFERENZIALE);
        //printf("%d: %f\n", i, dp->risultati_prova.corrente_scarica_batt[i] + offset_tot_medio); fflush(stdout);
    }

    printf("- Tensione media a vuoto:\t%.3fV\t===\n", tensione_media_vuoto); fflush(stdout);
    printf("- Corrente media no offset:\t%.3fA\t===\n", corrente_scarica_media / dp->risultati_prova.punti_corrente_scarica_batt); fflush(stdout);
    printf("- Corrente media con offset:\t%.3fA\t===\n", corrente_scarica_media_offset); fflush(stdout);
    printf("- Capacita' finale:\t\t%.3fAh\t===\n- Durata:\t\t\t%ds\t===\n", capacita_finale, dp->risultati_prova.punti_corrente_scarica_batt); fflush(stdout);
    

    creazione_pagina_web(capacita_finale, tensione_media_vuoto, capacita_finale_offset, corrente_scarica_media / dp->risultati_prova.punti_corrente_scarica_batt, corrente_scarica_media_offset, dp->risultati_prova.punti_corrente_scarica_batt);
    

    
    //FILE* fp_output = fopen("output.html", "w");
    //fprintf(fp_output, "- Capacita' finale:             %.3f===\n- Tensione media a vuoto:       %.3f===\n- Capacita' finale con offset:  %.3f===\n", capacita_finale, tensione_media_vuoto, capacita_finale_offset);
    //fprintf(fp_output, "%s", pagina_web);


    /* - Capacita' finale senza considerare offset 
       - Capacita' finale con offset
       - Tensione a vuoto --> tensione di partenza
       - Corrente media di scarica
       - Durata scarica (hh:mm:ss e in ssss)
       - Durata totale prova (inutile e sempre diverso)
       - % scarica rispetto al valore minimo e massimo attesi (4.3Ah e 4.5Ah)
       - Wh scaricati --> poi dividere per 3.6 (define)
       - Graficare il tutto
    */
}

/*void calcoli_temporali(tempi_prova* t){

    t->durata_prova = sottrai_date(t->start_prova, t->end_prova);
    t->durata_scarica = sottrai_date(t->start_scarica, t->end_scarica);
    t->durata_scarica_sec = data_in_secondi(t->durata_scarica);

}

tempi_prova sottrai_date (data d1, data d2){
    tempi_prova t;
    float d1s, d2s;
    int temp;
    
    //all'interno dell funzione d1 e' la data piu grande
    //controllo quale data e' piu grande
    if(d1.ore < d2.ore){
        //la d1 e' piu piccola di d2
        temp = d1.ore;
        d1.ore = d2.ore;
        d2.ore = temp;
    }else if((d1.ore == d2.ore) && (d1.minuti < d2.minuti)){
        temp = d1.minuti;
        d1.minuti = d2.minuti;
        d2.minuti = temp;
    }else if((d1.ore == d2.ore) && (d1.minuti == d2.minuti) && (d1.secondi < d2.secondi)){
        temp = d1.secondi;
        d1.secondi = d2.secondi;
        d2.secondi = temp;
    }

    d1s = data_in_secondi(d1); d2s = data_in_secondi(d2);
    somma = d1s - d2s;
    
    return secondi_in_data(somma);       
}

float data_in_secondi (data d){
    return (d.ore * 3600 + d.minuti * 60 + d.secondi);
}

tempi_prova secondi_in_data (float s){
    tempi_prova t;
    t.ore = 0; t.minuti = 0; t.secondi = 0;

    if(s >= 3600){
        t.ore = s / 3600;
        if((s % 3600) >= 60){
            t.minuti = s % 3600 - 60;
            t.ore++;
        }else{
            t.minuti = s % 3600;
        }
    }else{
        t.minuti = s /60;
        if((s % 60) >= 60){
            t.secondi = s % 60 - 60;
            t.minuti++;
        }else{
            t.secondi = s % 60;
        }
    }
    return t;     
}
*/

int max (int n1, int n2){
    if(n1 < n2){
        return n1;
    }else{
        return n2;
    }
}

void creazione_pagina_web (float f1, float f2, float f3, float f4, float f5, int i1){
    FILE* fp_output = fopen("output.html", "w");
    char pagina_web[10000], web1[150], web2[150], web3[150], web4[150], web5[150], web6[150];

    sprintf(pagina_web, "<!DOCTYPE html>\n");
    strcat(pagina_web, "<html lang=\"it\">\n");
    strcat(pagina_web, "<head>\n");
    strcat(pagina_web, "    <meta charset=\"UTF-8\">\n");
    strcat(pagina_web, "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    strcat(pagina_web, "    <title>Risultati Prova</title>\n");
    strcat(pagina_web, "    <style>\n");
    
    strcat(pagina_web, "        h1 {\n");
    strcat(pagina_web, "            color: darkblue;\n");
    strcat(pagina_web, "            font-size: 100px;\n");
    strcat(pagina_web, "            text-align: left;\n");
    strcat(pagina_web, "        }\n");
    
    
    strcat(pagina_web, "        body {\n");
    strcat(pagina_web, "            display: legacy;\n");
    strcat(pagina_web, "            justify-content: flex-start;\n");
    strcat(pagina_web, "            align-items: left;\n");
    strcat(pagina_web, "            height: 100vh;\n");
    strcat(pagina_web, "            background-color: #f8f8f8;\n");
    strcat(pagina_web, "            font-family: Arial, sans-serif;\n");
    strcat(pagina_web, "        }\n");
    strcat(pagina_web, "        .container {\n");
    strcat(pagina_web, "            display: grid;\n");
    strcat(pagina_web, "            grid-template-columns: 5000px;\n");
    strcat(pagina_web, "            gap: 0.8px;\n");
    strcat(pagina_web, "            text-align: left;\n");
    strcat(pagina_web, "            margin-top: 5px;\n");
    strcat(pagina_web, "        }\n");
    strcat(pagina_web, "        .item {\n");
    strcat(pagina_web, "            font-size: 60px;\n");
    strcat(pagina_web, "            font-weight: bold;\n");
    strcat(pagina_web, "            color: red;\n");
    strcat(pagina_web, "            margin: 80px;\n");
    strcat(pagina_web, "            justify-content: flex-start;\n");
    strcat(pagina_web, "            opacity: 0;\n");
    strcat(pagina_web, "            transform: scale(0.5);\n");
    strcat(pagina_web, "            animation: fadeInScale 1s forwards;\n");
    strcat(pagina_web, "        }\n");
    strcat(pagina_web, "        .char {\n");
    strcat(pagina_web, "            font-size: 60px;\n");
    strcat(pagina_web, "            font-weight: bold;\n");
    strcat(pagina_web, "            color: black;\n");
    strcat(pagina_web, "            margin: 5px;\n");
    strcat(pagina_web, "            opacity: 0;\n");
    strcat(pagina_web, "            transform: scale(0.5);\n");
    strcat(pagina_web, "            animation: fadeInScale 1s forwards;\n");
    strcat(pagina_web, "        }\n");
    strcat(pagina_web, "        .item:nth-child(1) { animation-delay: 0.2s; }\n");
    strcat(pagina_web, "        .item:nth-child(2) { animation-delay: 0.2s; }\n");
    strcat(pagina_web, "        .item:nth-child(3) { animation-delay: 0.2s; }\n");
    strcat(pagina_web, "        .item:nth-child(4) { animation-delay: 0.2s; }\n");
    strcat(pagina_web, "        .char:nth-child(1)   { animation-delay: 0.2s; }\n");
    strcat(pagina_web, "        @keyframes fadeInScale {\n");
    strcat(pagina_web, "            from {\n");
    strcat(pagina_web, "                opacity: 0;\n");
    strcat(pagina_web, "                transform: scale(0.5);\n");
    strcat(pagina_web, "            }\n");
    strcat(pagina_web, "            to {\n");
    strcat(pagina_web, "                opacity: 1;\n");
    strcat(pagina_web, "                transform: scale(1);\n");
    strcat(pagina_web, "            }\n");
    strcat(pagina_web, "        }\n");
    strcat(pagina_web, "    </style>\n");
    strcat(pagina_web, "</head>\n");
    strcat(pagina_web, "<body>\n");
    strcat(pagina_web, "   <h1>Risultati Prova CW 112W</h1>\n");
    strcat(pagina_web, "   <div class=\"container\">\n");
    sprintf(web1, "        <div class=\"item\"><span class=\"char\">Capacità finale: </span> <span>                    %.3f Ah</span></div>\n", f1);
    strcat(pagina_web, web1);
    sprintf(web2, "        <div class=\"item\"><span class=\"char\">Capacità finale con offset corrente: </span> <span>%.3f Ah</span></div>\n", f3);
    strcat(pagina_web, web2);
    sprintf(web3, "        <div class=\"item\"><span class=\"char\">Tensione iniziale media a vuoto: </span> <span>    %.3f V</span></div>\n", f2);
    strcat(pagina_web, web3);
    sprintf(web4, "        <div class=\"item\"><span class=\"char\">Corrente scarica media no offset: </span> <span>   %.3f A</span></div>\n", f4);
    strcat(pagina_web, web4);
    sprintf(web5, "        <div class=\"item\"><span class=\"char\">Corrente scarica media con offset: </span> <span>  %.3f A</span></div>\n", f5);
    strcat(pagina_web, web5);
    sprintf(web6, "        <div class=\"item\"><span class=\"char\">Durata scarica: </span> <span>%ds</span></div>\n", i1);
    strcat(pagina_web, web6);
    strcat(pagina_web, "   </div>\n");
    strcat(pagina_web, "</body>\n");
    strcat(pagina_web, "</html>\n");

    //printf("==================================\n%s\n================================", pagina_web); fflush(stdout);
    
    fprintf(fp_output, "%s", pagina_web);
    fclose(fp_output);
}