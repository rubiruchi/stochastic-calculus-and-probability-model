#include <cstdio>
#include <random>
// user-defined library
#include "../utils/poisson.h"
#include "../utils/parse_arg.h"
#include "../utils/rand_gen.h"
#include "../utils/event.h"

int main(int argc,char *argv[]) {
  // create parsing args object
  parse_args *args = new parse_args();
  // set rules
  args->set_args_rules("p","Probability of Poisson X","0.5","float");
  args->set_args_rules("l","lambda of Poisson N","3","int");
  args->set_args_rules("k","upperbound of k (for N, which N can derive to N1, N2 according to p and 1-p)","10000","int");
  args->set_args_rules("s","simulation times","10000","int");
  // parse it!
  args->parsing(argc,argv);

  double p = std::atof(args->get_args_val("p").val.c_str());
  double l = std::atof(args->get_args_val("l").val.c_str());
  int upperbound = std::atoi(args->get_args_val("k").val.c_str());
  int simulation_time = std::atoi(args->get_args_val("s").val.c_str());

  // Write file
  FILE *fp;
  fp=fopen("output/part_b.output","w+");
  // Set upperbound for gnuplot to use
  fprintf(fp,"# %d\n",upperbound);

  // ===================================== Mathematic Part =====================================
  // Calculate
  for(double i=1; i<=upperbound; i++) {
    // Get total P{X+Y=i}
    double p_s,p_xy=0;
    p_s = poisson(l,i);
    for(int j=0; j<=i; j++) {
      p_xy += poisson(l*p,j)*poisson(l*(1-p),(i-j));
    }
    fprintf(fp,"%lf %lf %lf\n",i,p_s,p_xy);
  }

  // ===================================== Simulation Part =====================================
  // discrete event simulation
  rand_gen *gen = new rand_gen();
  event_list *elist = new event_list();
  int rnt=simulation_time;
  FILE *fp_sim = fopen("output/part_b_sim.output","w+");

  std::default_random_engine generator;
  std::exponential_distribution<double> dist_1(l*p),dist_2(l*(1-p));

  event_type S;
  // init with rand
  elist->set(0,std::string("X"));
  elist->set(0,std::string("Y"));


  while(rnt) {
    // pop out
    if(elist->get(S)) {
      // Using probability P to decide which event will be push
      if(gen->frand(0,l)>(l*p)) {
        // become Y event
        elist->set(dist_2(generator),std::string("Y"));
      } else {
        elist->set(dist_1(generator),std::string("X"));
      }
      // record
      elist->record(S);
      rnt--;
    }
  }

  // Then eliminate first event (init)
  elist->rec.erase(elist->rec.begin(),elist->rec.begin()+2);

  // check
  int count=0,count_x=0,count_y=0;
  double slot=exp(-1.0/l),record_slot=slot,record_slot_x=exp(-1.0/(l*p)),record_slot_y=exp(-1.0/(l*(1-p)));
  std::map<int,int> counter,counter_x,counter_y;

  while(elist->rec.size()!=0) {
    event_type tmp;
    tmp=elist->rec.front();
    elist->rec.erase(elist->rec.begin(),elist->rec.begin()+1);
    // total: S=X+Y
    if(tmp.timestamp<=record_slot)
      count++;
    else {
      record_slot+=slot;
      counter[count]++;
      count=0;
    }
    // And then do X mode
    if(tmp.type==std::string("X")) {
      if(tmp.timestamp<=record_slot_x)
        count_x++;
      else {
        record_slot_x+=slot;
        counter_x[count_x]++;
        count_x=0;
      }
    }
    if(tmp.type==std::string("Y")) {
      if(tmp.timestamp<=record_slot_y)
        count_y++;
      else {
        record_slot_y+=slot;
        counter_y[count_y]++;
        count_y=0;
      }
    }

  }

  if(count!=0)
    counter[count]++;
  if(count_x!=0)
    counter_x[count_x]++;
  if(count_y!=0)
    counter_y[count_y]++;


  count=0,count_x=0,count_y=0;

  for(auto&it : counter) {
    count+=it.second;
  }

  for(auto&it : counter_x) {
    //printf("X: %d %d\n",it.first,it.second);
    count_x+=it.second;
  }

  for(auto&it : counter_y) {
    //printf("Y: %d %d\n",it.first,it.second);
    count_y+=it.second;
  }

  // Record the parameter we use for this time
  fprintf(fp_sim,"# %d %lf %lf %d %d %d %f\n",simulation_time,l,slot,count,count_x,count_y,p);

  for(auto&it : counter) {
    double p_xy=0.0;
    for(int i=0; i<=it.first; i++) {
      // S=X+Y, X:0~it.first, Y:it.first~0
      p_xy+=((float)counter_x[i]/(float)count_x)*((float)counter_y[it.first-i]/(float)count_y);
    }

    fprintf(fp_sim,"%d %lf %lf\n",it.first,(float)it.second/(float)count,p_xy);
  }

  return 0;
}