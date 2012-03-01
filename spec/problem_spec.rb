require 'spec_helper'


describe(RubyLinear::Problem) do
  
  describe 'load_file' do
    it 'should load a libsvm format file' do
      problem = RubyLinear::Problem.load_file(File.dirname(__FILE__) + '/fixtures/dna.scale.txt', -1)
      problem.bias.should == -1
      
      problem.l.should == 2000
      problem.n.should == 180
      
      #line 1 from the file
      problem.feature_vector(0).should == [[2,1], [7,1], [12,1], [15,1], [17,1], [23,1], [26,1], [28,1], [33,1], [34,1], [40,1], [45,1], [47,1], [50,1], [52,1], [58,1], [63,1], [64,1], [67,1], [72,1], [73,1], [76,1], [80,1], [83,1], [85,1], [88,1], [91,1], [95,1], [97,1], [101,1], [113,1], [120,1], [122,1], [126,1], [132,1], [138,1], [144,1], [145,1], [150,1], [151,1], [154,1], [160,1], [163,1], [170,1], [172,1], [177,1], [178,1]] 

      problem.labels.length.should == 2000
    end
    
    it 'should add 1 to the feature count if the bias is > 0' do
      problem = RubyLinear::Problem.load_file(File.dirname(__FILE__) + '/fixtures/dna.scale.txt', 1)
      
      problem.l.should == 2000
      problem.n.should == 181

      problem.feature_vector(0).should == [[2,1], [7,1], [12,1], [15,1], [17,1], [23,1], [26,1], [28,1], [33,1], [34,1], [40,1], [45,1], [47,1], [50,1], [52,1], [58,1], [63,1], [64,1], [67,1], [72,1], [73,1], [76,1], [80,1], [83,1], [85,1], [88,1], [91,1], [95,1], [97,1], [101,1], [113,1], [120,1], [122,1], [126,1], [132,1], [138,1], [144,1], [145,1], [150,1], [151,1], [154,1], [160,1], [163,1], [170,1], [172,1], [177,1], [178,1],[181,1]] 

    end
  end
  
  describe 'destroy' do
  
    it 'should release associated memory' do
      problem = RubyLinear::Problem.load_file(File.dirname(__FILE__) + '/fixtures/dna.scale.txt', 1)
      problem.destroyed?.should be_false
      problem.destroy!
      problem.destroyed?.should be_true
    end
      
  end
  
  describe 'new' do
    before(:each) do
      @samples = [ {2=>0.1,3=>0.3,4=>-1.2},{1=>0.4},{2=>0.1, 4=>1.4,5=>0.5},{1 => -0.1, 2=> -0.2, 3=>0.1,4=>1.1,5=>0.1}]
      @labels = [2,1,2,3]
      @max_feature = 5
    end
    
    context 'when the bias is < 0' do
      it 'should create a new problem' do
        problem = RubyLinear::Problem.new(@labels, @samples, -1, @max_feature)
        
        problem.l.should == 4
        problem.n.should == 5
        problem.bias.should == -1
        problem.labels.should == [2,1,2,3]
        problem.feature_vector(0).should == [[2,0.1], [3,0.3], [4,-1.2]]
      end
    end
    
    context 'when the bias is > 0' do
      it 'should add a  bias term to each vextor' do
        problem = RubyLinear::Problem.new(@labels, @samples, 1.0, @max_feature)
        #all the feature vectors should also be padded with
        problem.l.should == 4
        problem.n.should == 6
        problem.bias.should == 1
        problem.feature_vector(0).should == [[2,0.1], [3,0.3], [4,-1.2], [6,1]]
      end
    end
  end
end