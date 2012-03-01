require 'spec_helper'


describe(RubyLinear::Parameter) do
  
  describe('new') do
    it 'should default c to 1' do
      RubyLinear::Parameter.new(RubyLinear::L1R_L2LOSS_SVC).c.should == 1
    end

    it 'should default eps to 1' do
      RubyLinear::Parameter.new(RubyLinear::L1R_L2LOSS_SVC).eps.should == 0.01
    end

    it 'should set solver_type' do
      RubyLinear::Parameter.new(RubyLinear::L1R_L2LOSS_SVC).solver_type.should ==  RubyLinear::L1R_L2LOSS_SVC
    end

    it 'should default to overriding no weights' do
      RubyLinear::Parameter.new(RubyLinear::L1R_L2LOSS_SVC).weights.should == {}
    end
  end
end