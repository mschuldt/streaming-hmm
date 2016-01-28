//This provides code common to all versions of the classification programs.
//It reads values from .csv files and runs tests

fp_t *acc_reading;

fp_t* read_line(char* line){
  char* tok;
  tok = strtok(line, ",");//throw out time
  acc_reading = (fp_t*)malloc(sizeof(fp_t)*3);//mem leak
  for (int i = 0; i < 3; i++){
    tok = strtok(NULL, ",\n");
    acc_reading[i] = d2fp(strtod(tok, NULL));
  }

  return acc_reading;
}

int classify_csv_file(const char* filename){
  FILE* f = fopen(filename, "r");
  char line[1024];
  fgets(line, 1024, f);
  while (fgets(line, 1024, f)){
    input_reading(read_line(line));
  }
  fclose(f);
  return input_end();
}


int ok = 0;
int total = 0;

void check(const char* file, int expect){
  int g = classify_csv_file(file);
  if (g == expect){
    ok+=1;
  }else{
    printf("FAIL -  expected: %d, got: %d\n", expect, g);
  }
  total += 1;
}

int main(){
  init_models();

  dir_filter_ref_initial = (fp_t*)malloc(sizeof(fp_t)*3);
  dir_filter_ref = dir_filter_ref_initial;

  int n_states = models[0]->numStates;//assume they are all the same

  acc_reading = (fp_t*)malloc(sizeof(fp_t)*3);

  for (int i = 0; i < n_models; i++){
    models[i]->f = (fp_t*)calloc(n_states, sizeof(fp_t));
    models[i]->s = (fp_t*)calloc(n_states, sizeof(fp_t));
    models[i]->started = false;
  }

  //check("test.csv", 0);
  //exit(1);

  /*
  check("../we-g/gesture_recordings/square2/1.csv", 0);
  check("../we-g/gesture_recordings/square2/5.csv", 0);
  check("../we-g/gesture_recordings/square2/10.csv", 0);
  check("../we-g/gesture_recordings/square2/14.csv", 0);
  check("../we-g/gesture_recordings/square2/15.csv", 0);
  check("../we-g/gesture_recordings/square2/16.csv", 0);

  check("../we-g/gesture_recordings/up_down/1.csv", 1);
  check("../we-g/gesture_recordings/up_down/5.csv", 1);
  check("../we-g/gesture_recordings/up_down/10.csv", 1);
  check("../we-g/gesture_recordings/up_down/14.csv", 1);
  check("../we-g/gesture_recordings/up_down/15.csv", 1);

  check("../we-g/gesture_recordings/z/1.csv", 2);
  check("../we-g/gesture_recordings/z/5.csv", 2);
  check("../we-g/gesture_recordings/z/10.csv", 2);
  check("../we-g/gesture_recordings/z/14.csv", 2);

  check("../we-g/gesture_recordings/roll_flip/1.csv", 3);
  check("../we-g/gesture_recordings/roll_flip/5.csv", 3);
  check("../we-g/gesture_recordings/roll_flip/10.csv", 3);
  check("../we-g/gesture_recordings/roll_flip/14.csv", 3);
  check("../we-g/gesture_recordings/roll_flip/15.csv", 3);
  check("../we-g/gesture_recordings/roll_flip/16.csv", 3);
  */


  // -- 60%
  check("../we-g/raw_gesture_recordings_converted/circle/5.csv", 0);
  check("../we-g/raw_gesture_recordings_converted/circle/8.csv", 0);
  check("../we-g/raw_gesture_recordings_converted/circle/14.csv", 0);
  check("../we-g/raw_gesture_recordings_converted/circle/13.csv", 0);
  check("../we-g/raw_gesture_recordings_converted/circle/19.csv", 0);

  // -- 90%
  //check("../we-g/raw_gesture_recordings_converted/up-down/5.csv", 1);
  //check("../we-g/raw_gesture_recordings_converted/up-down/8.csv", 1);
  //check("../we-g/raw_gesture_recordings_converted/up-down/14.csv", 1);
  //check("../we-g/raw_gesture_recordings_converted/up-down/13.csv", 1);
  //check("../we-g/raw_gesture_recordings_converted/up-down/19.csv", 1);

  // -- 50%
  check("../we-g/raw_gesture_recordings_converted/flip-roll/5.csv", 2);
  check("../we-g/raw_gesture_recordings_converted/flip-roll/8.csv", 2);
  check("../we-g/raw_gesture_recordings_converted/flip-roll/14.csv", 2);
  check("../we-g/raw_gesture_recordings_converted/flip-roll/13.csv", 2);
  check("../we-g/raw_gesture_recordings_converted/flip-roll/19.csv", 2);


  printf("recognition rate: %d/%d, %f%%\n", ok, total, ((float)ok/(float)total)*100);

}
