#include "linear.h"
#include "tron.h"
#include "ruby.h"
#include <errno.h>
#include <ctype.h>
#ifdef __cplusplus
extern "C" {
#endif  
  
  
VALUE mRubyLinear;
VALUE cProblem;
VALUE cModel;

static void model_free(void *p){
  struct model * m = (struct model *)p;
  free_and_destroy_model(&m);
}

static VALUE model_load_file(VALUE klass, VALUE path){
  path = rb_str_to_str(path);
  struct model * model = load_model(rb_string_value_cstr(&path));
  VALUE tdata = Data_Wrap_Struct(klass, 0, model_free, model);
  return tdata;
}

static VALUE model_write_file(VALUE self,VALUE path){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  path = rb_str_to_str(path);
  save_model(rb_string_value_cstr(&path), model);
  return self;
}

static VALUE model_new(VALUE klass, VALUE r_problem, VALUE parameters){

  struct model *model = NULL;
  struct problem *problem;
  Data_Get_Struct(r_problem, struct problem, problem);
  struct parameter param;
  
  if(!problem->base){
    rb_raise(rb_eArgError, "problem has been disposed");
    return Qnil;
  }
  
  rb_funcall(mRubyLinear, rb_intern("validate_options"), 1, parameters);
  VALUE v;
  
  if(!NIL_P(v = rb_hash_aref(parameters, ID2SYM(rb_intern("eps"))))){
    param.eps = RFLOAT_VALUE(rb_to_float(v));
  }else{
    param.eps = 0.01;
  }

  if(!NIL_P(v = rb_hash_aref(parameters, ID2SYM(rb_intern("c"))))){
    param.C = RFLOAT_VALUE(rb_to_float(v));
  }else{
    param.C = 1;
  }

  v = rb_hash_aref(parameters, ID2SYM(rb_intern("solver")));
  param.solver_type = FIX2INT(v);

  if(!NIL_P(v = rb_hash_aref(parameters, ID2SYM(rb_intern("weights"))))){
    Check_Type(v, T_HASH);
    param.nr_weight = RHASH_SIZE(v);
    param.weight = (double*)calloc(param.nr_weight,sizeof(double));
    param.weight_label = (int*)calloc(param.nr_weight,sizeof(int));

    VALUE weights_as_array = rb_funcall(v, rb_intern("to_a"),0);

    for(long i=0; i < RARRAY_LEN(weights_as_array); i++){
      VALUE pair = RARRAY_PTR(weights_as_array)[i];
      VALUE label = RARRAY_PTR(pair)[0];
      VALUE weight = RARRAY_PTR(pair)[1];

      param.weight[i] = RFLOAT_VALUE(rb_to_float(weight));
      param.weight_label[i] = FIX2INT(label);
    }
    
  }else{
    param.nr_weight = 0;
    param.weight = NULL;
    param.weight_label = NULL;
  }

  
  const char *error_string = check_parameter(problem, &param);
  if(error_string){
    rb_raise(rb_eArgError, "%s", error_string);
    destroy_param(&param);
    return Qnil;
  }
  model = train(problem, &param);
  VALUE tdata = Data_Wrap_Struct(klass, 0, model_free, model);  
  destroy_param(&param);
  return tdata;
}
static VALUE model_feature_count(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  return INT2FIX(model->nr_feature);
}

static VALUE model_class_count(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  return INT2FIX(model->nr_class);
}

static VALUE model_class_bias(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  return rb_float_new(model->bias);
}

static VALUE model_destroy(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  free_model_content(model);
  model->w = NULL;
  model->label = NULL;
  return Qnil;
}

static VALUE model_destroyed(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  return model->w ? Qfalse : Qtrue;
}


struct feature_node * convert_ruby_sample_to_feature_node(struct model * model, VALUE data){
  Check_Type(data, T_HASH);
  VALUE pairs = rb_funcall(data,rb_intern("to_a"),0);
  int node_count = RARRAY_LEN(pairs) + (model->bias > 0 ? 2 : 1);
  struct feature_node * nodes = (struct feature_node *)calloc(node_count, sizeof(struct feature_node));
  
  int position = 0;
  for(int i=0; i < RARRAY_LEN(pairs); i++, position++){
    VALUE pair = RARRAY_PTR(pairs)[i];
    VALUE key = RARRAY_PTR(pair)[0];
    VALUE weight = RARRAY_PTR(pair)[1];
    
    nodes[i].index = FIX2INT(key);
    nodes[i].value = RFLOAT_VALUE(rb_to_float(weight));
  }
  if(model->bias > 0){
    nodes[position].index = model->nr_feature+1;
    nodes[position].value = model->bias;
    position++;
  }
  /*sentinel value*/
  nodes[position].index = -1;
  nodes[position].value = -1;
  return nodes;
}

static VALUE model_predict_values(VALUE self, VALUE data){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  
  if(!model->w){
    rb_raise(rb_eArgError, "model has been destroyed");
    return Qnil;
  }
  struct feature_node * nodes = convert_ruby_sample_to_feature_node(model, data);

  double *values = (double*)calloc(sizeof(double), model->nr_class);
  int label=predict_values(model, nodes, values);

  VALUE label_to_value_hash = rb_hash_new();
  for(int i = 0; i < model->nr_class; i++){
    int label = model->label[i];
    double value = values[i];
    rb_hash_aset(label_to_value_hash, INT2FIX(label), rb_float_new(value));
  }
  free(values);
  
  VALUE result = rb_ary_new();
  rb_ary_push(result, INT2FIX(label));
  rb_ary_push(result, label_to_value_hash);
  return result;
}

static VALUE model_predict(VALUE self, VALUE data){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  
  if(!model->w){
    rb_raise(rb_eArgError, "model has been destroyed");
    return Qnil;
  }
  struct feature_node * nodes = convert_ruby_sample_to_feature_node(model, data);
  int result = predict(model, nodes); 
  free(nodes);
  return INT2FIX(result);
}

static VALUE model_inspect(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  
  return rb_sprintf("#<RubyLinear::Model:%p classes:%d features:%d bias:%f>",(void*)self,model->nr_class, model->nr_feature,model->bias);
}

static VALUE model_labels(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  VALUE result = rb_ary_new();
  for(int i=0; i < model->nr_class; i++){
    rb_ary_store(result, i, INT2FIX(model->label[i]));
  }
  return result;
  
}

static VALUE model_solver(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);
  return INT2FIX(model->param.solver_type);
}


static VALUE model_weights(VALUE self){
  struct model *model;
  Data_Get_Struct(self, struct model, model);

  int n;
  if(model->bias>=0){
    n=model->nr_feature+1;
  }
  else{
    n=model->nr_feature;
  }
  int w_size = n;
  int nr_w;
  if(model->nr_class==2 && model->param.solver_type != MCSVM_CS){
    nr_w = 1;
  }
  else{
    nr_w = model->nr_class;
  }
  
  int weight_count = w_size*nr_w;
  
  VALUE result = rb_ary_new();
  for(int i=0; i < weight_count; i++){
    rb_ary_store(result, i, rb_float_new(model->w[i]));
  }
  return result;
}



static void problem_free(void *p) {
  struct problem * pr = (struct problem*)p;

  free(pr->y);
  free(pr->base);
  free(pr);
}

void exit_input_error(int line_num)
{
  rb_raise(rb_eArgError, "Wrong input format at line %d\n", line_num);
}

static char *line = NULL;
static int max_line_len;

static char* readline(FILE *input)
{
  int len;
  
  if(fgets(line,max_line_len,input) == NULL)
    return NULL;

  while(strrchr(line,'\n') == NULL)
  {
    max_line_len *= 2;
    line = (char *) realloc(line,max_line_len);
    len = (int) strlen(line);
    if(fgets(line+len,max_line_len-len,input) == NULL)
      break;
  }
  return line;
}

static VALUE problem_load_file(VALUE klass, VALUE path, VALUE bias){
  path = rb_str_to_str(path);
  /* lifted from train.c*/
  int max_index, inst_max_index, i;
  long int elements, j;
  FILE *fp = fopen(rb_string_value_cstr(&path),"r");
  char *endptr;
  char *idx, *val, *label;

  if(fp == NULL)
  {
    rb_sys_fail("can't open input file");
    return Qnil;
  }

  struct problem *prob = (struct problem*) calloc(1, sizeof(struct problem));
  VALUE tdata = Data_Wrap_Struct(klass, 0, problem_free, prob);
  prob->bias = RFLOAT_VALUE(rb_to_float(bias));
  prob->l = 0;
  elements = 0;
  max_line_len = 1024;
  line = (char*)calloc(sizeof(char),max_line_len);
  while(readline(fp)!=NULL)
  {
    char *p = strtok(line," \t"); // label

    // features
    while(1)
    {
      p = strtok(NULL," \t");
      if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
        break;
      elements++;
    }
    elements++; // for bias term
    prob->l++;
  }
  rewind(fp);


  prob->y = (int*)calloc(sizeof(int),prob->l);
  prob->x = (struct feature_node **)calloc(sizeof(struct feature_node *),prob->l);
  prob->base = (struct feature_node *)calloc(sizeof(struct feature_node),elements + prob->l);

  max_index = 0;
  j=0;
  for(i=0;i<prob->l;i++)
  {
    inst_max_index = 0; // strtol gives 0 if wrong format
    readline(fp);
    prob->x[i] = &prob->base[j];
    label = strtok(line," \t\n");
    if(label == NULL){ // empty line
      exit_input_error(i+1);
      fclose(fp);
      return Qnil;
    }
    prob->y[i] = (int) strtol(label,&endptr,10);
    if(endptr == label || *endptr != '\0'){
      exit_input_error(i+1);
      fclose(fp);
      return Qnil;
    }
    while(1)
    {
      idx = strtok(NULL,":");
      val = strtok(NULL," \t");

      if(val == NULL)
        break;

      errno = 0;
      prob->base[j].index = (int) strtol(idx,&endptr,10);
      if(endptr == idx || errno != 0 || *endptr != '\0' || prob->base[j].index <= inst_max_index){
        exit_input_error(i+1);
        fclose(fp);
        return Qnil;
      }
      else
        inst_max_index = prob->base[j].index;

      errno = 0;
      prob->base[j].value = strtod(val,&endptr);
      if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr))){
        exit_input_error(i+1);
        fclose(fp);
        return Qnil;
      }

      ++j;
    }

    if(inst_max_index > max_index)
      max_index = inst_max_index;

    if(prob->bias >= 0)
      prob->base[j++].value = prob->bias;

    prob->base[j++].index = -1;
  }

  if(prob->bias >= 0)
  {
    prob->n=max_index+1;
    for(i=1;i<prob->l;i++)
      (prob->x[i]-2)->index = prob->n; 
    prob->base[j-2].index = prob->n;
  }
  else
    prob->n=max_index;

  fclose(fp);
  return tdata;
}

static VALUE problem_new(VALUE klass, VALUE labels, VALUE samples, VALUE bias, VALUE attr_count){
  VALUE argv[4] = {labels, samples,bias,attr_count};
  
  struct problem  *ptr = (struct problem *)calloc(sizeof(struct problem),1);
  VALUE tdata = Data_Wrap_Struct(klass, 0, problem_free, ptr);
  rb_obj_call_init(tdata, 4, argv);
  return tdata;
}

static VALUE problem_labels(VALUE self){
  if(RTEST(rb_funcall(self, rb_intern("destroyed?"),0))){
    rb_raise(rb_eArgError, "problem has been destroyed");
    return Qnil;
  }
  
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);

  VALUE result = rb_ary_new();
  
  for( int i=0; i< problem -> l; i++){
    rb_ary_push(result, INT2FIX(problem->y[i]));
  }
  return result;

}

static VALUE problem_feature_vector(VALUE self, VALUE r_index){
  if(RTEST(rb_funcall(self, rb_intern("destroyed?"),0))){
    rb_raise(rb_eArgError, "problem has been destroyed");
    return Qnil;
  }
  
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  
  int index = FIX2INT(r_index);
  if(index >= problem->l){
    rb_raise(rb_eArgError, "index out of bounds");
    return Qnil;
  }
  VALUE result = rb_ary_new();
  
  for( struct feature_node *current = problem->x[index];current->index != -1; current++){
    VALUE pair = rb_ary_new();
    rb_ary_push(pair, INT2FIX(current->index));
    rb_ary_push(pair, rb_float_new(current->value));
    rb_ary_push(result, pair);
  }
  return result;
}


static VALUE problem_destroy(VALUE self){  
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  free(problem->base);
  problem->base = NULL;
  return self;
}

static VALUE problem_destroyed(VALUE self){  
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  return problem->base ? Qfalse : Qtrue;
}


static VALUE problem_inspect(VALUE self){
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  
  return rb_sprintf("#<RubyLinear::Problem:%p samples:%d features:%d bias:%f>",(void*)self,problem->l, problem->n,problem->bias);
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


static VALUE problem_bias(VALUE self){
  struct problem *problem;
  Data_Get_Struct(self, struct problem, problem);
  return rb_float_new(problem->bias);
}


static void addSample(struct problem * problem, int label, double weight){
  if(label > problem->n){
    rb_raise(rb_eArgError, "tried to add sample %d, %f, inconsistent with max feature of %d", label, weight, problem->n);
  }
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

  
  /* copy the y values  and calculate how many samples to allocate*/
  int required_feature_nodes = 0;
  int extra_samples = problem->bias > 0 ? 2 : 1; /*always 1 (the sentinel element, and possibly +1 for bias)*/
  for(int i=0; i<problem->l; i++){
    VALUE hash = RARRAY_PTR(samples)[i];
    problem->y[i] = FIX2INT(RARRAY_PTR(labels)[i]);
    required_feature_nodes += RHASH_SIZE(hash) + extra_samples; 
  }
  
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

extern int info_on;

static VALUE info_on_get(VALUE self) {
  return info_on ? Qtrue:Qfalse;
}

static VALUE info_on_set(VALUE self, VALUE new_value){
  info_on = RTEST(new_value) ? 1 : 0;
  return new_value;
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


  rb_define_singleton_method(mRubyLinear, "info_on", RUBY_METHOD_FUNC(info_on_get), 0);
  rb_define_singleton_method(mRubyLinear, "info_on=", RUBY_METHOD_FUNC(info_on_set), 1);

  
  cProblem = rb_define_class_under(mRubyLinear, "Problem", rb_cObject);
  rb_define_singleton_method(cProblem, "new", RUBY_METHOD_FUNC(problem_new), 4);
  rb_define_singleton_method(cProblem, "load_file", RUBY_METHOD_FUNC(problem_load_file),2);
  rb_define_method(cProblem, "initialize", RUBY_METHOD_FUNC(problem_init), 4);
  rb_define_method(cProblem, "l", RUBY_METHOD_FUNC(problem_l), 0);
  rb_define_method(cProblem, "n", RUBY_METHOD_FUNC(problem_n), 0);
  rb_define_method(cProblem, "bias", RUBY_METHOD_FUNC(problem_bias), 0);
  rb_define_method(cProblem, "feature_vector", RUBY_METHOD_FUNC(problem_feature_vector), 1);
  rb_define_method(cProblem, "labels", RUBY_METHOD_FUNC(problem_labels), 0);
  rb_define_method(cProblem, "destroy!", RUBY_METHOD_FUNC(problem_destroy), 0);
  rb_define_method(cProblem, "destroyed?", RUBY_METHOD_FUNC(problem_destroyed), 0);
  rb_define_method(cProblem, "inspect", RUBY_METHOD_FUNC(problem_inspect), 0);

  cModel = rb_define_class_under(mRubyLinear, "Model", rb_cObject);
  rb_define_singleton_method(cModel, "load_file", RUBY_METHOD_FUNC(model_load_file), 1);
  rb_define_singleton_method(cModel, "new", RUBY_METHOD_FUNC(model_new), 2);
  rb_define_method(cModel, "save", RUBY_METHOD_FUNC(model_write_file), 1);
  rb_define_method(cModel, "predict", RUBY_METHOD_FUNC(model_predict), 1);
  rb_define_method(cModel, "predict_values", RUBY_METHOD_FUNC(model_predict_values), 1);
  rb_define_method(cModel, "destroy!", RUBY_METHOD_FUNC(model_destroy), 0);
  rb_define_method(cModel, "destroyed?", RUBY_METHOD_FUNC(model_destroyed), 0);
  rb_define_method(cModel, "inspect", RUBY_METHOD_FUNC(model_inspect), 0);
  rb_define_method(cModel, "labels", RUBY_METHOD_FUNC(model_labels), 0);
  rb_define_method(cModel, "solver", RUBY_METHOD_FUNC(model_solver), 0);
  rb_define_method(cModel, "weights", RUBY_METHOD_FUNC(model_weights), 0);

  rb_define_method(cModel, "feature_count", RUBY_METHOD_FUNC(model_feature_count), 0);
  rb_define_method(cModel, "class_count", RUBY_METHOD_FUNC(model_class_count), 0);
  rb_define_method(cModel, "bias", RUBY_METHOD_FUNC(model_class_bias), 0);


}




#ifdef __cplusplus
} /* extern "C" */
#endif