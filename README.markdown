Ruby bindings for [liblinear](http://www.csie.ntu.edu.tw/~cjlin/liblinear/)

Usage
=====

Loading a problem in the libsvm format

    RubyLinear::Problem.load_file("/path/to/file",bias)
    
Defining a problem from an array of samples

    samples = [{1 => 1, 2=> 0.2}, {3 => 1, 4=> 0.2}, {2 => 1, 3 => 0.3}]
    max_feature = samples.map {|sample| sample.keys.max}.max
        
    labels = [1,2,1]
    RubyLinear::Problem.new labels, samples, 1.0, max_feature
    
Your sample can of course be sparse: you only need to name features with a non-zero associated value
    

Loading a model from a file
  
    RubyLinear::Model.load_file('/path/to/file')
    

Training a model from parameters and a problem

    parameter = RubyLinear::Parameter.new(RubyLinear::L1R_L2LOSS_SVC)
    parameter.eps = 0.01 #these are the defaults
    parameter.c = 1
    
    RubyLinear::Model.new(problem, parameter)
    
Predicting a value

   sample = {1 => 0.3, 4 => 0.1}
   model.predict(sample)