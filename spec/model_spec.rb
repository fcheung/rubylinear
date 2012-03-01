require 'spec_helper'


describe(RubyLinear::Model) do
  
  let :test_vector do#first line from dna.scale.t
    {6 => 1, 7 => 1, 11 => 1, 18 => 1, 20 => 1, 24 => 1, 27 => 1, 30 => 1, 33 => 1, 34 => 1, 38 => 1, 42 => 1, 45 => 1, 47 => 1, 53 => 1, 60 => 1, 61 => 1, 65 => 1, 69 => 1, 70 => 1, 75 => 1, 78 => 1, 79 => 1, 84 => 1, 87 => 1, 88 => 1, 92 => 1, 99 => 1, 101 => 1, 103 => 1, 108 => 1, 110 => 1, 112 => 1, 119 => 1, 123 => 1, 124 => 1, 128 => 1, 131 => 1, 134 => 1, 137 => 1, 139 => 1, 142 => 1, 147 => 1, 149 => 1, 156 => 1, 157 => 1, 161 => 1, 164 => 1, 166 => 1, 171 => 1, 173 => 1, 180 => 1}
  end
  
  describe('load_file') do
    before(:each) do
      @model = RubyLinear::Model.load_file(File.dirname(__FILE__) + '/fixtures/dna.dat')
    end
    
    it 'should load the model from the path' do
      @model.bias.should == 1
      @model.class_count.should == 3
      @model.feature_count.should == 180
      @model.labels.should == [3,1,2]
    end
    
    it 'should be able to predict' do
      @model.predict(test_vector).should == 3
    end
  end
  
  describe('new') do
    let(:problem) {RubyLinear::Problem.load_file(File.dirname(__FILE__) + '/fixtures/dna.scale.txt', 1.0)}
    it 'should train the model from the parameters and the problem' do
      m = RubyLinear::Model.new(problem, :solver => RubyLinear::L1R_L2LOSS_SVC)
      m.predict(test_vector).should == 3
    end
    
    context 'when unknwon options are presented' do
      it 'should raise argument error' do
        expect { RubyLinear::Model.new(problem, :solver => RubyLinear::L1R_L2LOSS_SVC, :bogus_option => true) }.to raise_error(ArgumentError)
        
      end
      
    end
  end
end