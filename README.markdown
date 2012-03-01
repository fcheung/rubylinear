Ruby bindings for [liblinear](http://www.csie.ntu.edu.tw/~cjlin/liblinear/)

Quick Start
=====

### Loading a problem in the libsvm format

    RubyLinear::Problem.load_file("/path/to/file",bias)
    
### Defining a problem from an array of samples

    samples = [{1 => 1, 2=> 0.2}, {3 => 1, 4=> 0.2}, {2 => 1, 3 => 0.3}]
    max_feature = samples.map {|sample| sample.keys.max}.max
        
    labels = [1,2,1]
    RubyLinear::Problem.new labels, samples, 1.0, max_feature
    
Your sample can of course be sparse: you only need to name features with a non-zero associated value
    

### Loading a model from a file
  
    RubyLinear::Model.load_file('/path/to/file')
    

### Training a model from parameters and a problem
    
    RubyLinear::Model.new(problem, :solver => RubyLinear::L1R_L2LOSS_SVC)

### Changing the default parameters    

    RubyLinear::Model.new(problem, :solver => RubyLinear::L1R_L2LOSS_SVC, :c => 1.1, :eps => 0.02, :weights => {2 => 0.9})
    #use C=1.1, eps = 0.02 and apply a weight of 0.9 to class 2
    
### Predicting a value

   sample = {1 => 0.3, 4 => 0.1}
   model.predict(sample)
   

What is this
============

Liblinear is a linear classifier. From its home page:

> LIBLINEAR is a linear classifier for data with millions of instances and features. It supports
> L2-regularized classifiers 
> L2-loss linear SVM, L1-loss linear SVM, and logistic regression (LR)
> L1-regularized classifiers (after version 1.4) 
> L2-loss linear SVM and logistic regression (LR)

In a nutshell if you provide a bunch of examples and what classes they should fall into, Liblinear will predict what classes future datapoints should fall in. Examples include classifying text into spam or not spam, sorting news articles into the list of topics or determining whether a tweet is expressing something positive or negative.

## Classifying text

If you are classifying text, you first need to break up your text into tokens. This might be individual words, pairs of words (bigrams) or triples etc. Each one of these is a feature. For example if the 3 pieces of text in our training set were

    s1 = "Hello world"
    s2 = "Bonjour"
    s3 = "Guten tag"
    s4 = "tag world"
    
then our dictionary contains the words `Hello, world, Bonjour, Guten, tag` and so s1 has features 1,2 s2 has feature 3, s3 has features 4,5 and s4  has features 2,5. You need to keep hold of the mapping of these words to feature numbers - You'll need this later on.

You must then assign labels: these are the classes you are trying to sort things into. For example 1 is english, 2 is french and 3 is german. The code to setup such a sample set is

    samples = [
      { 1 => 1, 2 => 1},
      { 3 => 1},
      { 4 => 1, 5 => 1},
      { 2 => 1, 5 => 1}
    ]
    labels = [1,2,3,1]

    problem = RubyLinear::Problem.new labels, samples, 1.0, 5

The 3rd parameter is the bias term - for now ignore it. the last parameter is the maximum feature value. Here we've just used 1 to indicate the presence of a feature and nothing to indicate its abscence. A more sophisticated approach would have different values depending on how relevant the feature is, for example by using tf-idf. You can also load problem data in the libsvm format.

    model = RubyLinear::Model.new(problem, :solver_type => RubyLinear::L1R_L2LOSS_SVC)
    
will create and train a model from your sample data. Liblinear provides a bunch of different solvers: `RubyLinear::L2R_LR`, `RubyLinear::L2R_L2LOSS_SVC_DUAL`, `RubyLinear::L2R_L2LOSS_SVC`, `RubyLinear::L2R_L1LOSS_SVC_DUAL`, `RubyLinear::MCSVM_CS`, `RubyLinear::L1R_L2LOSS_SVC`, `RubyLinear::L1R_LR`, `RubyLinear::L2R_LR_DUAL`. You can also set liblinear's C and eps options (the defaults are 1 and 0.01 respectively)

    model = RubyLinear::Model.new(problem, :solver_type => RubyLinear::L1R_L2LOSS_SVC, :c => 1, :eps => 0.01)
    
Models can be saved to disk (`model.save(path_to_file)`) and loaded (`RubyLinear::Model.load_file(path_to_file)`). These use the standard liblinear formats so you should be able to use models trained with the liblinear command line tools.

Once you have a model you can feed it samples and it will return the class it thinks is the best match. To do this you split the text to be classified in the same way that you did in the sample process and map it to feature numbers in the same way. If you have a word which isn't in your mapping (ie you come across a word which wasn't in your training set, just ignore it). For example if the text to test was "Hello bob", you would do

    model.predict({1 => 1})
    
Feature 1 is present and bob is an unknown word and so gets dropped

