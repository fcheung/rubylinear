#include "linear.h"
#include "tron.h"
#include "ruby.h"

#ifdef __cplusplus
extern "C" {
#endif  
  
  
VALUE mRubyLinear;
VALUE cProblem;
VALUE cModel;

static void problem_free(void *p) {
  struct problem * pr = (struct problem*)p;

  free(pr->y);
  free(pr->base);
  free(pr);
}  

static VALUE problem_new(VALUE klass, VALUE labels, VALUE samples, VALUE bias, VALUE attr_count){
  VALUE argv[4] = {labels, samples,bias,attr_count};
  
  struct problem  *ptr = (struct problem *)calloc(sizeof(struct problem),1);
  VALUE tdata = Data_Wrap_Struct(klass, 0, problem_free, ptr);
  rb_obj_call_init(tdata, 4, argv);
  return tdata;
}



static VALUE problem_l(VALUE self){
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  return INT2FIX(problem->l);
}

static VALUE problem_n(VALUE self){
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  return INT2FIX(problem->n);
}


static void addSample(struct problem * problem, int label, double weight){
  problem->base[problem->offset].index = label;
  problem->base[problem->offset].value = weight;
  problem->offset++;
}
static VALUE addSampleIterator(VALUE yielded_object, VALUE context, int argc, VALUE argv[]){
  struct problem *problem;
  Data_Get_Struct(context, struct problem, problem);
  VALUE key = RARRAY_PTR(yielded_object)[0];
  VALUE value = RARRAY_PTR(yielded_object)[1];
  
  int label = FIX2INT(key);
  double weight = RFLOAT_VALUE(rb_to_float(value));
  addSample(problem, label, weight);
  return Qnil;
}


static VALUE problem_init(VALUE self, VALUE labels, VALUE samples, VALUE bias, VALUE r_attr_count){
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  
  labels = rb_check_array_type(labels);
  samples = rb_check_array_type(samples);
  problem->bias = RFLOAT_VALUE(rb_to_float(bias));

  problem->n = FIX2INT(r_attr_count);
  if(problem->bias > 0){
    problem->n += 1;
  }
  
  if(RARRAY_LEN(labels) != RARRAY_LEN(samples)){
    rb_raise(rb_eArgError, "samples and labels were of different length (%lu, %lu)", RARRAY_LEN(labels), RARRAY_LEN(samples));
    return Qnil;
  }
  problem->l = RARRAY_LEN(samples);
  problem->y = (int*)calloc(sizeof(int), problem->l);

  
  /* copy the y values. At the same time we work out what the number of labels is (which is the maximum observed labels) and how many samples to allocate*/
  int required_feature_nodes = 0;
  int extra_samples = problem->bias > 0 ? 2 : 1; /*always 1 (the sentinel element, and possibly +1 for bias)*/
  for(int i=0; i<problem->l; i++){
    VALUE hash = RARRAY_PTR(samples)[i];
    problem->y[i] = FIX2INT(RARRAY_PTR(labels)[i]);
    if(problem->y[i] > problem->l){
      problem->l = problem->y[i];
    }
    required_feature_nodes += RHASH_SIZE(hash) + extra_samples; 
  }
  
  printf("required_feature_nodes is %d\n",required_feature_nodes);
  problem->offset = 0;
  problem->base = (struct feature_node *)calloc(sizeof(struct feature_node), required_feature_nodes);
  problem->x = (struct feature_node **)calloc(sizeof(struct feature_node*), problem->l);
  /* copy the samples */

  ID each = rb_intern("each");
  for(int i=0; i< problem->l; i++){
    VALUE hash = RARRAY_PTR(samples)[i];
    problem->x[i] = problem->base + problem->offset;
    rb_block_call(hash, each,0,NULL, RUBY_METHOD_FUNC(addSampleIterator),self);
    if(problem->bias>0){
      addSample(problem,problem->n,problem->bias);
    }
    addSample(problem,-1,-1);
  }
  if(problem->offset != required_feature_nodes){
    printf("allocated %d feature_nodes but used %d\n", required_feature_nodes, problem->offset);
    
  }
  return self;
}
void Init_rubylinear_native() {
  mRubyLinear = rb_define_module("RubyLinear");
  
  rb_define_const(mRubyLinear, "L2R_LR", INT2FIX(L2R_LR));
  rb_define_const(mRubyLinear, "L2R_L2LOSS_SVC_DUAL", INT2FIX(L2R_L2LOSS_SVC_DUAL));
  rb_define_const(mRubyLinear, "L2R_L2LOSS_SVC", INT2FIX(L2R_L2LOSS_SVC));
  rb_define_const(mRubyLinear, "L2R_L1LOSS_SVC_DUAL", INT2FIX(L2R_L1LOSS_SVC_DUAL));
  rb_define_const(mRubyLinear, "MCSVM_CS", INT2FIX(MCSVM_CS));
  rb_define_const(mRubyLinear, "L1R_L2LOSS_SVC", INT2FIX(L1R_L2LOSS_SVC));
  rb_define_const(mRubyLinear, "L1R_LR", INT2FIX(L1R_LR));
  rb_define_const(mRubyLinear, "L2R_LR_DUAL", INT2FIX(L2R_LR_DUAL));
  
  cModel = rb_define_class_under(mRubyLinear, "Model", rb_cObject);
  cProblem = rb_define_class_under(mRubyLinear, "Problem", rb_cObject);
  rb_define_singleton_method(cProblem, "new", RUBY_METHOD_FUNC(problem_new), 4);
  rb_define_method(cProblem, "initialize", RUBY_METHOD_FUNC(problem_init), 4);
  rb_define_method(cProblem, "l", RUBY_METHOD_FUNC(problem_l), 0);
  rb_define_method(cProblem, "n", RUBY_METHOD_FUNC(problem_n), 0);
}




#ifdef __cplusplus
} /* extern "C" */
#endif