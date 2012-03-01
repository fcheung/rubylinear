require 'spec_helper'


describe(RubyLinear) do
  
  # The data files are the data set from http://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass.html#dna
  # The model was created with train -s 5 -e 0.01 -B 1 dna.scale.txt dna.dat
  # the output was created with predict dna.scale.t dna.dat dna.out
  
  def parse_line(l)
    h = {}
    l.split(' ').each do |s|
      if s =~ /(\d+):(\d+)/
        h[$1.to_i] = $2.to_i
      end
    end
    h
  end
  
  before(:each) do
    problem = RubyLinear::Problem.load_file("spec/fixtures/dna.scale.txt",1)
    @model = RubyLinear::Model.new(problem, :solver => RubyLinear::L1R_L2LOSS_SVC)
  end
  
  it 'should produce the same output as the command line tools' do    
    @model.should predict_values('spec/fixtures/dna.out').for_input('spec/fixtures/dna.scale.t')
  end
  
  RSpec::Matchers.define :predict_values do |output_file|
    match do |model|
      input_lines = File.readlines(@input)
      output_lines = File.readlines(output_file)
      raise "mismatched inputs" unless input_lines.length == output_lines.length
      raise "input empty" if input_lines.length < 1
      input_lines.each_with_index do |line, index|
        sample = parse_line(line)
        if (@predicted = model.predict(sample)) != (@actual = output_lines[index].to_i)
          @failed_line = index + 1
          break
        end
      end
      @failed_line.nil?
    end
    
    failure_message_for_should do |actual|
      "Incorrect prediction on line #{@failed_line}, expected #{@actual} got #{@predicted}"
    end
    
    chain :for_input do |input|
      @input = input
    end
  end
  
end