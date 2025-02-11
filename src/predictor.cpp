//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Hariram Ramakrishnan ";
const char *studentID = "A69030260";
const char *email = "hramakrishnan@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 17; // Number of bits used for Global History
int bpType;            // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// bimodal
uint8_t *bht_bimodal;

// gshare
uint8_t *bht_gshare;
uint64_t ghistory;

// tournament

const int tournament_ghr_width = 16;
const int tournament_chooser_width = 16;
const int tournament_local_pht_width = 10;

uint16_t *tournament_bht_local;
uint8_t *tournament_pht_local;

uint8_t *tournament_pht_global;
uint16_t tournament_ghr;

uint8_t *tournament_pht_chooser;

// perceptron
// 17-b perceptrons
// 2^16 perceptrons in total
const int perceptron_ghr_width = 63;
const int perceptron_weight_bias_width = 64;

const int perceptron_table_size = 12;

int perceptron_table[1 << perceptron_table_size][perceptron_weight_bias_width];
int perceptron_ghr[perceptron_ghr_width];

// tournament - perceptron x local PHT Predictor - PLT
const int plt_chooser_width = 12;
const int plt_local_pht_width = 10;

const int plt_ghr_width = 16;
const int plt_weight_bias_width = 17;

const int plt_perceptron_table_size = 12;

int plt_perceptron_table[1 << perceptron_table_size][perceptron_ghr_width];
int plt_perceptron_ghr[perceptron_ghr_width];

uint16_t plt_bht_local[1 << plt_local_pht_width];
uint8_t plt_pht_local[1 << plt_local_pht_width];

uint8_t plt_pht_chooser[1 << plt_chooser_width];
uint16_t plt_chooser_ghr;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor


//bimodal predictor functions 
void init_bimodal()
{
  int biomdal_entries = 1 << 18;
  bht_bimodal = (uint8_t *)malloc(biomdal_entries * sizeof(uint8_t));
  for (int i = 0;i < biomdal_entries;i++){
    bht_bimodal[i] = WN;
  }
}

uint8_t bimodal_predict(uint32_t pc) {
  uint32_t bht_entries = 1 << 18;

  // Gets the last 17-bits of the PC
  uint32_t pc_lower_bits = pc & (bht_entries - 1);

  switch (bht_bimodal[pc_lower_bits])
  {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in Bimodal BHT!\n");
    return NOTTAKEN;
  }
}

void train_bimodal(uint32_t pc, uint8_t outcome)
{
  uint32_t bht_entries = 1 << 18;
    
  // Gets the last 17-bits of the PC
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t index = pc_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_bimodal[index])
  {
  case WN:
    bht_bimodal[index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    bht_bimodal[index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    bht_bimodal[index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    bht_bimodal[index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Bimodal BHT!\n");
    break;
  }

}

void cleanup_bimodal()
{
  free(bht_bimodal);
}

// gshare functions
void init_gshare()
{
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for (i = 0; i < bht_entries; i++)
  {
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t gshare_predict(uint32_t pc)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch (bht_gshare[index])
  {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    return NOTTAKEN;
  }
}

void train_gshare(uint32_t pc, uint8_t outcome)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index])
  {
  case WN:
    bht_gshare[index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    bht_gshare[index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    break;
  }

  // Update history register
  ghistory = ((ghistory << 1) | outcome);
}

void cleanup_gshare()
{
  free(bht_gshare);
}

// tournament predictor functions ----------------------------------------------

void init_tournament(){
  
  //printf("local pht %d",pht_size);

  int local_pht_size = 1 << tournament_local_pht_width;

  int global_pht_size = 1 << tournament_ghr_width;

  int chooser_size = 1 << tournament_chooser_width;


  tournament_bht_local = (uint16_t *)malloc(local_pht_size * sizeof(uint16_t));
  tournament_pht_local = (uint8_t *)malloc(local_pht_size * sizeof(uint8_t));

  tournament_ghr = 0;

  tournament_pht_global = (uint8_t *)malloc(global_pht_size * sizeof(uint8_t));

  // chooser based chooser width
  tournament_pht_chooser = (uint8_t *)malloc(chooser_size * sizeof(uint8_t));


  for (int i = 0; i < local_pht_size; i++) {
    tournament_pht_local[i] = WN;
    tournament_bht_local[i] = 0;  
  }
  for (int i = 0; i < global_pht_size; i++) {
    tournament_pht_global[i] = WN; 
  }
  for (int i = 0; i < chooser_size; i++) {
    tournament_pht_chooser[i] = 2;  
  }

}

uint8_t tournament_predict_local(uint32_t pc){
  uint32_t bht_entries = 1 << tournament_local_pht_width;
  // Gets the last 10-bits of the PC
  uint32_t local_bht_index = pc & ((1 << tournament_local_pht_width) - 1);

  uint16_t current_pattern = tournament_bht_local[local_bht_index];
  uint16_t current_pattern_10bits = current_pattern & (bht_entries - 1);

  switch (tournament_pht_local[current_pattern_10bits])
  {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in Tournament local PHT! %d\n",tournament_pht_local[current_pattern_10bits]);
    return NOTTAKEN;
  }

}

uint8_t tournament_predict_global(uint32_t pc){
  // Update history register - NOT NEEDED HERE
  //tournament_ghr = ((tournament_ghr << 1) | outcome);
  
  //Index global history with 12-b global pattern
  //int ghr_size = 1 << tournament_ghr_width;
  uint32_t tournament_ghr_bht_index = tournament_ghr & ((1 << tournament_ghr_width) - 1);

  switch(tournament_pht_global[tournament_ghr_bht_index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in Tournament Global HT!\n");
      return NOTTAKEN;
  }

}

uint8_t tournament_predict(uint32_t pc){
  // Uses the chooser
  uint8_t local = tournament_predict_local(pc);
  uint8_t global = tournament_predict_global(pc);

  int chooser_size = 1 << tournament_chooser_width;
  uint32_t tournament_chooser_index = tournament_ghr & (chooser_size - 1);

  /*
  Chooser 2b counter:
    0: global
    1: global
    2: local
    3: local
  */
  if(tournament_pht_chooser[tournament_chooser_index] >= 2) {
    return local;
  } else {
    return global;
  }
}

void train_tournament(int32_t pc, uint8_t outcome) {

  uint8_t local = tournament_predict_local(pc);
  uint8_t global = tournament_predict_global(pc);
  
  int chooser_size = 1 << tournament_chooser_width;
  uint32_t tournament_chooser_index = tournament_ghr & (chooser_size - 1);

  if(local == outcome && global != outcome){
    // Wrap around for 2-bit sat counter
    if(tournament_pht_chooser[tournament_chooser_index] >= 3){
      tournament_pht_chooser[tournament_chooser_index] = 3;
    } else {
      tournament_pht_chooser[tournament_chooser_index] += 1;
    }
  } else if(global == outcome && local != outcome) {
    if(tournament_pht_chooser[tournament_chooser_index] == 0){
      tournament_pht_chooser[tournament_chooser_index] = 0;
    }  
    else {
      tournament_pht_chooser[tournament_chooser_index] -= 1;
    }
  } 

  /*if(tournament_pht_chooser[tournament_chooser_index] > 3){
    //printf("invalid value %d\n",tournament_pht_chooser[tournament_chooser_index]);
    tournament_pht_chooser[tournament_chooser_index] = 3;
  }*/

  /*uint8_t chosen_prediction;
  if (tournament_pht_chooser[tournament_chooser_index] >= 2) {
      chosen_prediction = local;
  } else {
      chosen_prediction = global;
  }
  // ***Correct Chooser Update Logic***
  if (chosen_prediction == outcome) {  // If the *chosen* predictor was correct
    if (local == outcome && global != outcome) {
      tournament_pht_chooser[tournament_chooser_index] = std::min((uint8_t)3, tournament_pht_chooser[tournament_chooser_index] + 1);
    }
    } else {  // If the *chosen* predictor was incorrect
      if (local != outcome && global == outcome) {
          tournament_pht_chooser[tournament_chooser_index] = std::max((uint8_t)0, tournament_pht_chooser[tournament_chooser_index] - 1);
      }
    } */

  // get lower 10 bits of pc
  uint32_t bht_entries = 1 << tournament_local_pht_width;
  uint32_t tournament_local_index = pc & (bht_entries - 1);

  // Update branch history table entry
  uint16_t current_pattern = tournament_bht_local[tournament_local_index];
  uint16_t current_pattern_10bits = current_pattern & 0x3FF;

  switch (tournament_pht_local[current_pattern_10bits])
  {
  case WN:
    tournament_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    tournament_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    tournament_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    tournament_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Tournament Local PHT! %d\n",tournament_pht_local[current_pattern_10bits]);
    break;
  }

  uint32_t tournament_ghr_pht_index = tournament_ghr & ((1 << tournament_ghr_width) - 1);

  //Index global history with 12-b global pattern
  switch(tournament_pht_global[tournament_ghr_pht_index]){
    case WN:
    tournament_pht_global[tournament_ghr_pht_index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    tournament_pht_global[tournament_ghr_pht_index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    tournament_pht_global[tournament_ghr_pht_index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    tournament_pht_global[tournament_ghr_pht_index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Tournament Global BHT!\n");
    break;
  }

  tournament_bht_local[tournament_local_index] = ((tournament_bht_local[tournament_local_index] << 1) | outcome);

  // Update history register
  tournament_ghr = ((tournament_ghr << 1) | outcome);
  // Masking to correct width
  tournament_ghr = (tournament_ghr) & ((1 << tournament_ghr_width) - 1);
}

void cleanup_tournament(){
  free(tournament_pht_global);
  free(tournament_bht_local);
  free(tournament_pht_local);
  free(tournament_pht_chooser);
}

// perceptron functions ----------------------------------------------

void init_perceptron(){
  int perceptron_table_entries = 1 << perceptron_table_size;

  for (int i = 0; i < perceptron_table_entries; i++) {
    perceptron_table[i][0] = 1; // All bias = 1 initially
    for (int j = 1; j < perceptron_weight_bias_width; j++){
      perceptron_table[i][j] = 0;
    }
  }

  //perceptron_ghr[0] = 1; // Always 1
  for (int j = 0; j < perceptron_ghr_width; j++){
    perceptron_ghr[j] = 0;
  }
  
}

uint8_t perceptron_predict(uint32_t pc){
  uint32_t perceptron_entries = 1 << perceptron_table_size;
  
  // Gets the last 16-bits of the PC
  uint32_t perceptron_table_index = pc & (perceptron_entries - 1);

  int current_perc[perceptron_weight_bias_width];
  for(int i = 0;i<perceptron_weight_bias_width;i++){
    current_perc[i] = perceptron_table[perceptron_table_index][i];
  }

  int dot_product = 0;

  for(int i =0;i<perceptron_weight_bias_width;i++){
    if(i==0){
      dot_product += current_perc[i]; // Only bias
    } else{
      dot_product += current_perc[i] * perceptron_ghr[i-1];
    }
  }
  return dot_product > 0 ? TAKEN:NOTTAKEN;
}

void train_perceptron(int32_t pc, uint8_t outcome){
  uint32_t perceptron_entries = 1 << perceptron_table_size; // 2^16
  
  // Gets the last 16-bits of the PC
  uint32_t perceptron_table_index = pc & (perceptron_entries - 1);

  int current_perc[perceptron_weight_bias_width];
  for(int i = 0;i<perceptron_weight_bias_width;i++){
    current_perc[i] = perceptron_table[perceptron_table_index][i];
  }

  int dot_product = 0;

  for(int i =0;i<perceptron_weight_bias_width;i++){
    if(i==0){
      dot_product += current_perc[i]; // Only bias
    } else{
      dot_product += current_perc[i] * perceptron_ghr[i-1];
    }
  }

  int result = dot_product > 0 ? 1:-1;
  int result_T_NT = dot_product > 0 ? TAKEN:NOTTAKEN; // only to compare with outcome

  int t = (outcome == TAKEN) ? 1:-1;


  if(result_T_NT != outcome || abs(dot_product) <= 35 ){

      int bias = current_perc[0];
      int new_bias = bias + t;
      perceptron_table[perceptron_table_index][0] = new_bias;


      for(int i =1;i<perceptron_weight_bias_width;i++){
        int weight = current_perc[i];
        int new_weight = weight + t * perceptron_ghr[i-1];
         
        //Update weight back into actual perceptron table
        perceptron_table[perceptron_table_index][i] = new_weight;
      }
  }

  // Update GHR
  // perceptron_ghr[0] is alwyas 1
  for(int i=perceptron_ghr_width-1;i>0;i--){
    perceptron_ghr[i] = perceptron_ghr[i-1];
    //printf("%d",perceptron_ghr[i]);
  }
  perceptron_ghr[0] = (outcome == TAKEN) ? 1 : -1;
  //printf("%d",perceptron_ghr[0]);
  //printf("\n");

}

void cleanup_perceptron(){
}

// PLT predictor functions -------------------------------------------

void init_plt(){
  int perceptron_table_entries = 1 << plt_perceptron_table_size;

  for (int i = 0; i < perceptron_table_entries; i++) {
    plt_perceptron_table[i][0] = 1; // All bias = 1 initially
    for (int j = 1; j < plt_weight_bias_width; j++){
      plt_perceptron_table[i][j] = 0;
    }
  }

  plt_perceptron_ghr[0] = 1; // Always 1
  for (int j = 1; j < perceptron_ghr_width; j++){
    plt_perceptron_ghr[j] = 0;
  }
  plt_chooser_ghr = 0;

  int local_pht_size = 1 << plt_local_pht_width;
  int chooser_size = 1 << plt_chooser_width;

  for (int i = 0; i < local_pht_size; i++) {
    plt_pht_local[i] = WN;
    plt_bht_local[i] = 0;  
  }
  
  for (int i = 0; i < chooser_size; i++) {
    plt_pht_chooser[i] = 2;  
  }

}

uint8_t plt_local_predict(uint32_t pc){
  uint32_t bht_entries = 1 << plt_local_pht_width;
  // Gets the last 10-bits of the PC
  uint32_t local_bht_index = pc & ((1 << plt_local_pht_width) - 1);

  // Get pattern from BHT
  uint16_t current_pattern = plt_bht_local[local_bht_index];

  uint16_t current_pattern_10bits = current_pattern & (bht_entries - 1);

  switch (plt_pht_local[current_pattern_10bits])
  {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in Tournament local PHT! %d\n",plt_pht_local[current_pattern_10bits]);
    return NOTTAKEN;
  }
}

uint8_t plt_perceptron_predict(uint32_t pc){
  uint32_t perceptron_entries = 1 << plt_perceptron_table_size;
  
  // Gets the last 16-bits of the PC
  uint32_t perceptron_table_index = pc & (perceptron_entries - 1);

  int current_perc[plt_weight_bias_width];

  for(int i = 0;i<plt_weight_bias_width;i++){
    current_perc[i] = plt_perceptron_table[perceptron_table_index][i];
  }

  int dot_product = 0;

  for(int i =0;i<plt_weight_bias_width;i++){
    if(i==0){
      dot_product += current_perc[i]; // Only bias
    } else{
      dot_product += current_perc[i] * plt_perceptron_ghr[i-1];
    }
  }
  return dot_product > 0 ? TAKEN:NOTTAKEN;
}

uint8_t plt_predict(uint32_t pc){
    // Uses the chooser
    uint8_t local = plt_local_predict(pc);
    uint8_t perceptron = plt_perceptron_predict(pc);
  
    int chooser_size = 1 << plt_chooser_width;

    // chooserGHR - same as perceptron GHR - used as index for chooser
    uint32_t plt_chooser_index = plt_chooser_ghr & (chooser_size - 1);
  
    /*
    Chooser 2b counter:
      0: global
      1: global
      2: local
      3: local
    */
    if(plt_pht_chooser[plt_chooser_index] >= 2) {
      return local;
    } else {
      return perceptron;
    }
}

void train_plt(int32_t pc, uint8_t outcome) {
  uint8_t local = plt_local_predict(pc);
  uint8_t perceptron = plt_perceptron_predict(pc);
  
  int chooser_size = 1 << plt_chooser_width;

  // Using chooser GHR since chooser index in being accessed
  uint32_t plt_chooser_index = plt_chooser_ghr & (chooser_size - 1);

  if(local == outcome && perceptron != outcome){
    // Wrap around for 2-bit sat counter
    if(plt_pht_chooser[plt_chooser_index] >= 3){
      plt_pht_chooser[plt_chooser_index] = 3;
    } else {
      plt_pht_chooser[plt_chooser_index] += 1;
    }
  } else if(perceptron == outcome && local != outcome) {
    if(plt_pht_chooser[plt_chooser_index] == 0){
      plt_pht_chooser[plt_chooser_index] = 0;
    }  
    else {
      plt_pht_chooser[plt_chooser_index] -= 1;
    }
  } 


  // Updating local PHT predictor ----------------------------------------------

  // Extract index from PC
  uint32_t bht_entries = 1 << plt_local_pht_width;
  uint32_t plt_local_index = pc & (bht_entries - 1);

  // Extract pattern from BHT
  uint16_t current_pattern = plt_bht_local[plt_local_index];
  uint16_t current_pattern_10bits = current_pattern & 0x3FF;

  // Update state in PHT
  switch (plt_pht_local[current_pattern_10bits])
  {
  case WN:
    plt_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    plt_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    plt_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    plt_pht_local[current_pattern_10bits] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in PLT Local PHT! %d\n",plt_pht_local[current_pattern_10bits]);
    break;
  }

  
  
  // Updating Perceptron predictor ----------------------------------------------
  uint32_t perceptron_entries = 1 << plt_perceptron_table_size; // 2^16
  
  // Gets the last 16-bits of the PC - use as index in Perceptron table
  uint32_t perceptron_table_index = pc & (perceptron_entries - 1);

  // Extracting perceptron corresponding to PC
  int current_perc[plt_weight_bias_width];
  for(int i = 0;i<plt_weight_bias_width;i++){
    current_perc[i] = plt_perceptron_table[perceptron_table_index][i];
  }

  // Calculating dot product
  int dot_product = 0;
  for(int i =0;i<plt_weight_bias_width;i++){
    if(i==0){
      dot_product += current_perc[i]; // Only bias
    } else{
      dot_product += current_perc[i] * plt_perceptron_ghr[i-1];
    }
  }

  int result = dot_product > 0 ? 1:-1;
  int result_T_NT = dot_product > 0 ? TAKEN:NOTTAKEN; // only to compare with outcome
  int t = (outcome == TAKEN) ? 1:-1; // sstores correct outcome for weight updating

  // Updating weight and bias
  if(result_T_NT != outcome || abs(dot_product) <= 37 ){

      int bias = current_perc[0];
      int new_bias = bias + t;
      plt_perceptron_table[perceptron_table_index][0] = new_bias;


      for(int i =1;i<plt_weight_bias_width;i++){
        int weight = current_perc[i];
        int new_weight = weight + t * plt_perceptron_ghr[i-1];
         
        //Update weight back into actual perceptron table
        plt_perceptron_table[perceptron_table_index][i] = new_weight;
      }
  }

  // Update GHR
  // perceptron_ghr[0] is alwyas 1
  for(int i=plt_ghr_width-1;i>0;i--){
    plt_perceptron_ghr[i] = plt_perceptron_ghr[i-1];
  }
  plt_perceptron_ghr[0] = (outcome == TAKEN) ? 1 : -1;

  // Update chooser GHR 
  plt_chooser_ghr = ((plt_chooser_ghr << 1) | outcome);
  plt_chooser_ghr = (plt_chooser_ghr) & ((1 << plt_ghr_width) - 1);

  plt_bht_local[plt_local_index] = ((plt_bht_local[plt_local_index] << 1) | outcome);

}


void init_predictor()
{
  switch (bpType)
  {
  case STATIC:
    break;
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    init_tournament();
    break;
  case CUSTOM:
    //init_bimodal();
    init_perceptron();
    //init_plt();
    break;
  default:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct)
{

  // Make a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare_predict(pc);
  case TOURNAMENT:
    return tournament_predict(pc);
  case CUSTOM:
    //return bimodal_predict(pc);
    return perceptron_predict(pc);
    //return plt_predict(pc);
  default:
    break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome, uint32_t condition, uint32_t call, uint32_t ret, uint32_t direct)
{
  if (condition)
  {
    switch (bpType)
    {
    case STATIC:
      return;
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tournament(pc,outcome);
    case CUSTOM:
      //return train_bimodal(pc,outcome);
      return train_perceptron(pc,outcome);
      //return train_plt(pc,outcome);
    default:
      break;
    }
  }
}
