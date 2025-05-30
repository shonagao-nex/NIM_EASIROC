#!/bin/env ruby

ENV['INLINEDIR'] = File.dirname(File.expand_path(__FILE__))

require 'readline'
require 'optparse'
require 'bundler'
require 'bundler/setup'
require 'csv'
require 'yaml'
Bundler.require
require_relative './VME-EASIROC.rb'

Trigger = 10000 #10kHz

class CommandDispatcher
  DIRECT_COMMANDS = %i(ls rm rmdir cp mv mkdir cat less root sleep)
  DIRECT_COMMANDS_OPTION = {ls: "-h --color=auto", root: "-l"}
  COMMANDS = %w(shutdownHV setHV statusHV statusTemp statusInputDAC 
    checkHV setInputDAC setRegister setThreshold 
    increaseHV decreaseHV initialCheck 
    muxControl slowcontrol standby fit drawScaler dsleep
    read readloop adc tdc scaler cd pwd mode reset help version timeStamp 
    exit quit progress stop makeError ) + DIRECT_COMMANDS.map(&:to_s)

  
  def initialize(vmeEasiroc, hist, q)
    @vmeEasiroc = vmeEasiroc
    @hist = hist
    @q = q
  end

  def dispatch(line)
    command, *arg = line.split
    
    if !COMMANDS.include?(command)
      puts "unknown command #{command}"
      return
    elsif command=="progress" || command=="stop"
      puts "command #{command} works only while reading out data ... "
      return
    end
    
    begin
      send(command, *arg)
    rescue ArgumentError
      puts "invalid argument '#{arg.join(' ')}' for command '#{command}'"
    rescue => e
      puts e.message
      puts "exit..."
      setHV(0.0)
      sleep 1
      exit
    end
  end

  DIRECT_COMMANDS.each do |command|
    define_method(command) do |*arg|
      option = DIRECT_COMMANDS_OPTION[command]
      option ||= ''
      option << ' '
      system(command.to_s + ' ' + option + arg.join(' '))
    end
  end

  def dsleep(time)
    sleep time.to_f
  end

  def shutdownHV
    @vmeEasiroc.sendShutdownHV
  end

  def setHV(value)
    @vmeEasiroc.sendMadcControl
    @vmeEasiroc.sendHVControl(value.to_f)
  end

  def increaseHV(value)
    value = value.to_f
    if value < 0.0 then
      puts "Input value must be positive!"
      return
    elsif value > 90.0 then
      puts "Too large input value! Must be smaller than 90.0"
      return
    end

    count = value.div(10.0)
    for i in 0..count do
      setHV(i*10.0)
      sleep 1
      checkHV((i+1)*10.0, 20.0)
      sleep 0.2
    end
    setHV(value)
    sleep 1
    checkHV((count+1)*10.0, 20.0)
    sleep 0.2
  end

  def decreaseHV(value)
    @vmeEasiroc.sendMadcControl
    checkHV(value.to_f+10.0, 20.0)
    rd_madc = @vmeEasiroc.readMadc(3)
    count = rd_madc.div(10.0)
    for i in 0..count do
      setHV((count-i)*10.0)
      sleep 1
      checkHV((count-i+1)*10.0, 20.0)
      sleep 0.2
    end
    setHV(0.0)
    sleep 1
    checkHV(10.0, 20.0)
    sleep 0.2
  end

  def initialCheck
    @vmeEasiroc.sendMadcControl
    checkHV(10.0, 20.0)
    sleep 0.2
    setHV(2.0)
    sleep 1
    checkHV(10.0, 20.0)
    sleep 0.2
    setHV(3.0)
    sleep 1
    checkHV(10.0, 20.0)
    sleep 0.2
    setHV(5.0)
    sleep 1
    checkHV(10.0, 20.0)
    sleep 0.2
    setHV(10.0)
    sleep 1
    checkHV(20.0, 20.0)
    sleep 0.2
    setHV(0.0)
  end

  def statusHV
    @vmeEasiroc.sendMadcControl

    ## Read the MPPC bias voltage
    rd_madc = @vmeEasiroc.readMadc(3)
    puts sprintf("Bias voltage >> %.2f V", rd_madc)
    
    ## Read the MPPC bias current
    rd_madc = @vmeEasiroc.readMadc(4)
    puts sprintf("Bias current >> %.2f uA", rd_madc)
  end

  def statusTemp
    @vmeEasiroc.sendMadcControl
    
    ## Read the temparature1
    rd_madc = @vmeEasiroc.readMadc(5)
    puts sprintf("Temparature 1  >> %.2f C", rd_madc)

    ## Read the temparature2
    rd_madc = @vmeEasiroc.readMadc(0)
    puts sprintf("Temparature 2  >> %.2f C", rd_madc)
  end
 
  def statusInputDAC(channel, filename="temp")
    @vmeEasiroc.sendMadcControl
    
    ## Read the Input DAC voltage
    chInt = channel.to_i
    readNum = -1

    if 0<=chInt && chInt<=63
      num = chInt%32
      readNum = chInt/32 + 1
      @vmeEasiroc.setCh(num)
      rd_madc = @vmeEasiroc.readMadc(readNum)
      puts sprintf("ch %2d: Input DAC >> %.2f V",chInt,rd_madc)
    elsif chInt == 64
      puts "Reading monitor ADC..."
#      if /\.yml$/ !~ filename
#        filename << '.yml'
#      end

      status_filename = filename
#      if (!filename.include?("temp") && File.exist?(status_filename))
#        puts "#{status_filename} already exsit."
#        status_filename="status/temp_#{Time.now.to_i}.yml"
#      end
      puts "Save as #{status_filename}"

      status = {}
      status[:HV] = @vmeEasiroc.readMadc(3).round(3)
      status[:current] = @vmeEasiroc.readMadc(4).round(3)
      status[:Temp1] = @vmeEasiroc.readMadc(5).round(3)
      status[:Temp2] = @vmeEasiroc.readMadc(0).round(3)
      status[:InputDAC]=[]
      ch = 0..31
      ch.each{|eachnum|
        @vmeEasiroc.setCh(eachnum)
        status[:InputDAC] << @vmeEasiroc.readMadc(1).round(3)
      }
      ch = 32..63
      ch.each{|eachnum|
        @vmeEasiroc.setCh(eachnum-32)
        status[:InputDAC] << @vmeEasiroc.readMadc(2).round(3)
      }
      puts status
      File.write(status_filename, status.to_yaml)
    elsif chInt == 65
      status_filename = filename

      hv       = format('%.3f', @vmeEasiroc.readMadc(3))
      current  = format('%.3f', @vmeEasiroc.readMadc(4))
      temp1    = format('%.3f', @vmeEasiroc.readMadc(5))
      temp2    = format('%.3f', @vmeEasiroc.readMadc(0))

      status = {}
      status[:HV] = @vmeEasiroc.readMadc(3).round(3)
      status[:current] = @vmeEasiroc.readMadc(4).round(3)
      status[:Temp1] = @vmeEasiroc.readMadc(5).round(3)
      status[:Temp2] = @vmeEasiroc.readMadc(0).round(3)
      input_dac = []
      (0..15).each do |eachnum|
        @vmeEasiroc.setCh(eachnum)
        value = @vmeEasiroc.readMadc(1)
        input_dac << format('%.3f', value)
      end

      write_header = !File.exist?(status_filename)

      CSV.open(status_filename, 'a') do |csv|
        if write_header
          header = ["timestamp", "HV(V)", "current(uA)", "T1(degC)", "T2(degC)"] + (0..15).map { |i| "IDC#{i}" }
          csv << header
        end
#        row = [Time.now, status[:HV], status[:current], status[:Temp1], status[:Temp2]] + input_dac
        row = [Time.now, hv, current, temp1, temp2] + input_dac
        csv << row
      end

    else
      puts "channel: 0~63, or 64(all channels)"
      return
    end
    @vmeEasiroc.setCh(32)
  end

  def readInputs(output_csv, data_filename, duration)
    begin
      time = Time.now
      start_time_str = time.strftime("%Y-%m-%d %H:%M:%S")
      unix_time = time.to_i
  
      reg_config = YAML.load_file('yaml/RegisterValue.yml')
      inputdac_config = YAML.load_file('yaml/InputDAC.yml')

      easiroc1_regs = reg_config['EASIROC1']

      capacitor_hg_str  = easiroc1_regs['Capacitor HG PA Fdbck']
      time_constant_str = easiroc1_regs['Time Constant HG Shaper']
      dac_code_raw      = easiroc1_regs['DAC code']
      def extract_number(val)
        val.to_s[/\d+/].to_i
      end

      capacitor_hg  = extract_number(capacitor_hg_str)
      time_constant = extract_number(time_constant_str)
      dac_code      = extract_number(dac_code_raw)

      input_dac_array = inputdac_config.dig('EASIROC1', 'Input 8-bit DAC')
      first_16_input_dacs = input_dac_array[0, 16] if input_dac_array.is_a?(Array)
  
      csv_headers = ['FileName', 'StartTime', 'UnixTime', 'Duration(sec)', 'C_HG(fF)', 'Ctau(ns)', 'DAC'] +
                    (0...16).map { |i| "IDC#{i}" }
      csv_row = [data_filename, start_time_str, unix_time, duration, capacitor_hg, time_constant, dac_code] + first_16_input_dacs
  
      write_header = !File.exist?(output_csv) || File.zero?(output_csv)
  
      CSV.open(output_csv, 'a') do |csv|
        csv << csv_headers if write_header
        csv << csv_row
      end
  
    rescue => e
      puts "Failed to log run info to #{output_csv}: #{e.message}"
    end
  end

  def checkHV(vollim=62.0, curlim=20.0, repeat=3)
    @vmeEasiroc.sendMadcControl

    vollim = vollim.to_f
    curlim = curlim.to_f
    repeat = repeat.to_i
    check=false
    count=0
    while !check do
      count+=1
      if count > repeat then
        puts "Attempt limit. exit..."
        setHV(0.0)
        sleep 1
        exit
      end

      ## Read the MPPC bias voltage
      voltage = @vmeEasiroc.readMadc(3)

      ## Read the MPPC bias current
      current = @vmeEasiroc.readMadc(4)

      if voltage > vollim || current > curlim then
        puts sprintf("Over the limit. voltage=%.2fV, current=%.2fuA. trying again...\n", voltage, current)
        sleep 1
      else
        puts sprintf("Status OK. voltage=%.2fV, current=%.2fuA\n", voltage, current)
        check=true
      end
    end
  end

  def setInputDAC(voltage)
    @vmeEasiroc.setInputDAC(voltage.to_f)
    sleep 0.5
    slowcontrol
  end

  def setRegister(key, value)
    @vmeEasiroc.setRegister(key, value)
    sleep 0.5
    slowcontrol
  end
  
  def setThreshold(pe, chip="0", filename="temp")
    @vmeEasiroc.setThreshold(pe, chip, filename)
    sleep 0.5
    slowcontrol
  end

  def muxControl(chnum)
    @vmeEasiroc.setCh(chnum.to_i)
  end
  
  def slowcontrol
    @vmeEasiroc.reloadSetting
    @vmeEasiroc.sendSlowControl
    @vmeEasiroc.sendProbeRegister
    @vmeEasiroc.sendReadRegister
    @vmeEasiroc.sendPedestalSuppression
    @vmeEasiroc.sendSelectbaleLogic
    @vmeEasiroc.sendTriggerWidth
    @vmeEasiroc.sendTimeWindow
    @vmeEasiroc.sendUsrClkOutRegister 
#@vmeEasiroc.sendTriggerValues
  end
  
  def adc(on_off)
    puts "Set adc #{on_off}"
    if(on_off == 'on')
      @vmeEasiroc.sendAdc = true
    elsif(on_off == 'off')
      @vmeEasiroc.sendAdc = false
    else
      puts "Unknown argument #{on_off}"
      return
    end
  end
  
  def tdc(on_off)
    puts "Set tdc #{on_off}"
    if(on_off == 'on')
      @vmeEasiroc.sendTdc = true
    elsif(on_off == 'off')
      @vmeEasiroc.sendTdc = false
    else
      puts "Unknown argument #{on_off}"
      return
    end
  end
  
  def scaler(on_off)
    puts "Set scaler #{on_off}"
    if(on_off == 'on')
      @vmeEasiroc.sendScaler = true
    elsif(on_off == 'off')
      @vmeEasiroc.sendScaler = false
    else
      puts "Unknown argument #{on_off}"
      return
    end
  end

  def cd(path)
    begin
      Dir.chdir(path)
    rescue Errno::ENOENT
      puts "No such file or directry #{path}"
    end
  end

  def pwd
    puts Dir.pwd
  end

  def standby(counts)
    counts.to_i.times {
      buf = @q.pop
      puts "EASIROC #{buf} is ready."
    }
    $logger.debug "sleep 1 in standby."
    sleep 1
    $logger.debug "End of standby."
  end

  def read(events, filename="temp", mode="default")

    $logger.debug "Begin of read."
    events = events.to_i
    if /\.dat$/ !~ filename
      filename << '.dat'
    end

    data_filename = 'data/' + filename
    if (!filename.include?("temp") && File.exist?(data_filename))
      puts "#{data_filename} already exsits."
      data_filename="data/temp_#{Time.now.to_i}.dat"
    end
    puts "Save as #{data_filename}"

    if mode=="default"
      progress_bar = nil
      File.open(data_filename, 'wb') do |file|
        @vmeEasiroc.readEvent(events) do |header, data|
          progress_bar ||= ProgressBar.create(
            total: events,
            format: '%p%% [%b>%i] %c %revent/s %e'
          )
          file.write(header[:header])
          file.write(data.pack('N*'))
          progress_bar.increment
        end
      end
      progress_bar.finish

    elsif mode=="queue"
      File.open(data_filename, 'wb') do |file|
        $logger.debug "Create fork to readEvent."
        pid = Process.fork {
          @vmeEasiroc.readEvent(events) do |header, data|
            file.write(header[:header])
            file.write(data.pack('N*'))
          end
        }
        $logger.debug "Child process pid: #{pid}"
        @q.push(@vmeEasiroc.name)
        Process.waitpid pid
      end
      sleep 1

    elsif mode=="monitor"
      num_events = Queue.new
      send_stop = Queue.new

      readline_thread = Thread.new do
        sleep 1
        numEvent = 0
        progress_rd = 0.0
        commandsInRead = %w(progress stop statusHV statusTemp statusInputDAC)

        while buf_read = Readline.readline('DAQ is running... > ', true)
          buf_com, *buf_arg = buf_read.split
          if !commandsInRead.include?(buf_com)
            puts "Cannnot excute '#{buf_com}' while reading data..."
          elsif buf_com == "progress"
            numEvent = num_events.pop
            sleep 0.5
            progress_rd = numEvent.to_f/events*100
            puts sprintf("Number of events: %d, progress: %.3f%",numEvent,progress_rd)
          elsif buf_com == "stop"
            send_stop.push(1)
            sleep 5
          else
            dispatch(buf_read)
          end
        end
      end

      read_thread = Thread.new do
        ievents = 0
        File.open(data_filename, 'wb') do |file|
          @vmeEasiroc.readEvent(events) do |header, data|
            file.write(header[:header])
            file.write(data.pack('N*'))

            ievents += 1
            if !num_events.empty?
              num_events.pop
            end
            num_events.push(ievents)

            if !send_stop.empty?
              puts "Daq stop is requested"
              break
            end
          end
#          puts sprintf("!!!!Readout finished!!!! Total number of events: %d, %d%", ievents, ievents.to_f/events*100)
          puts sprintf("!!!!Readout finished!!!! Total number of events: %d", ievents)
          readline_thread.kill
        end
      end

      read_thread.join
      readline_thread.join

      num_events.clear
      send_stop.clear

    elsif mode=="enclk"
      setRegister("clk", "0")
      enclk = Queue.new

      begin
        enclk_thread = Thread.new {
          Thread.pass
          enclk.pop
          sleep 1
          setRegister("clk", "1")
        }
      rescue => e
        $logger.info "Error in read, mode==enclk, enclk_thread. name: #{@vmeEasiroc.name}"
        $logger.info e.message
      end

      begin
        timelimit=(events/Trigger).to_i
        read_thread = Thread.new {
          ievents = 0
          File.open(data_filename, 'wb') do |file|
            enclk.push(1)
            Thread.pass
            @vmeEasiroc.readEvent(events, timelimit+10) do |header, data|
              progress_bar ||= ProgressBar.create(
                total: events,
                format: '%p%% [%b>%i] %c %revent/s %e'
              )
              file.write(header[:header])
              file.write(data.pack('N*'))
              progress_bar.increment

              ievents += 1
            end
            if progress_bar.kind_of?(ProgressBar) then
              $logger.info "ProgressBar finish."
              progress_bar.finish
            end
          end
        }
        isTimeout = (read_thread.join(timelimit+20) == nil)
        if isTimeout then
          raise("Timeout Error. name: #{@vmeEasiroc.name}")
          read_thread.kill
        end
      rescue => e
        $logger.info "Error in read, mode==enclk, read_thread. name: #{@vmeEasiroc.name}"
        $logger.info e.message
      ensure
        $logger.info "Timeout Error. name: #{@vmeEasiroc.name}" if isTimeout
      end

      read_thread.join
      enclk_thread.join

      enclk.clear

      sleep 1
      setRegister("clk", "0")
    else
      puts "Invalid mode... 'default', 'monitor', or 'enclk'"
      return
    end

    if File.exist?(@hist) && FileTest::executable?(@hist)
      system("#{@hist} #{data_filename}")
    end
    slowcontrol
  end

  def readloop(arg = nil)
    puts "Start readloop (infinite monitor-mode loop). Type 'stop' to exit."
    Dir.mkdir('data') unless Dir.exist?('data')

    loop do
      # Initialization
      num_events = Queue.new
      send_stop = Queue.new
      duration = 1800   # duration in 1 run (second)
      stop_requested = false

      timestamp = Time.now.strftime("%Y%m%d_%H%M%S")
      data_filename = "data/run_#{timestamp}.dat"
      log_filename  = "data/run_#{timestamp}.log"
  
      puts "Start new DAQ, Data file: #{data_filename}"
      puts "Log file : #{log_filename}"
  
      puts "Reading monitor ADC..."
      2.times do
        statusInputDAC(65, log_filename)
        sleep 0.5
      end
  
      starttime = Time.now
      gpio_pid = spawn("python3 gpio_loop.py")
      Process.detach(gpio_pid)
      puts "Launched gpio_loop.py (PID=#{gpio_pid})"
 
      readline_thread = Thread.new do
        while buf_read = Readline.readline("DAQ is running... > ", true)
          cmd, *args = buf_read.split
          case cmd
          when "stop"
            send_stop.push(true)
            stop_requested = true
            puts "Stopping now....."
            break
          when "statusHV", "statusTemp", "statusInputDAC"
            dispatch(buf_read)
          else
            puts "Cannot execute '#{buf_read}' while reading data..."
          end
        end
      end
  
      read_thread = Thread.new do
        File.open(data_filename, 'wb') do |file|
          start_time = Time.now
          last_status_time = start_time
          ievents = 0
          progress_bar = ProgressBar.create(total: duration, format: '%p%% [%b>%i] %t, %a')

          @vmeEasiroc.readEvent(999_999_999) do |header, data|
            file.write(header[:header])
            file.write(data.pack('N*'))
  
            ievents += 1

            elapsed_time = Time.now - start_time
            progress_bar.progress = [elapsed_time, duration].min.to_i
            avg_rate = elapsed_time > 0 ? ievents / elapsed_time : 0.0
            progress_bar.title = "#{ievents} events, #{format('%.2f', avg_rate)} event/s"
 
            if Time.now - last_status_time >= 60
              statusInputDAC(65, log_filename)
              last_status_time = Time.now
            end

            begin
              send_stop.pop(true)
              puts "DAQ stop is requested"
              break
            rescue ThreadError
            # Queue is empty, continue
            end

            # Switching data file after duration time
            break if Time.now - start_time >= duration
          end
          progress_bar.finish if progress_bar
        end
      end
  
      read_thread.join
      readline_thread.kill if readline_thread.alive?
      read_thread.kill if read_thread.alive?
 
      puts "Stop DAQ #{data_filename} at #{timestamp}"

      runtime = Time.now - starttime
      runtime_str = format('%.1f', runtime)
      readInputs('data/runlist.csv',data_filename,runtime_str)

      puts "Reading monitor ADC..."
      2.times do
        statusInputDAC(65, log_filename)
        sleep 0.5
      end

      is_alive = system("ps -p #{gpio_pid} > /dev/null")
      if is_alive
        puts "Killing gpio_loop.py..."
        begin
          Process.kill("TERM", gpio_pid)
          sleep 1
          if system("ps -p #{gpio_pid} > /dev/null")
            puts "gpio_loop.py did not terminate, sending KILL"
            Process.kill("KILL", gpio_pid)
          end
        rescue => e
          puts "Error killing gpio_loop.py: #{e.message}"
        end
      end

      puts "Making archive..."
      archive_filename = data_filename.sub(/\.dat$/, '.tar.xz')
      system("tar -Jcf #{archive_filename} -C data #{File.basename(data_filename)}")
      puts

      puts "Making rootfile and pdf...."
      Dir.chdir("analysis") do
        makeroot_script = "makeroot.sh"
        online_script = "online.sh"
      
        if File.exist?(makeroot_script)
          system("./#{makeroot_script} #{data_filename}")
        else
          puts "#{makeroot_script} not found."
        end

        root_filename = data_filename.gsub('data/', 'rootfiles/').gsub('.dat', '.root')
        if File.exist?(online_script)
          system("./#{online_script} #{root_filename}")
        else
          puts "#{online_script} not found."
        end
      end

      slowcontrol
      # break after "stop" command
      break if stop_requested
      puts "Switching now, wait a moment"
      sleep 2
    end
  end

  def fit(filename="temp", *ch) 
    status_filename = "status/" + filename + ".yml"
    status = YAML.load_file(status_filename)
    if ch.empty? then
      64.times {|ich|
        voltage = status[:HV]-status[:InputDAC][ich]
        system(%Q(root -l -b -q 'fit1.cpp("#{filename}", #{voltage}, #{ich})'))
      } 
    else
      ch.map(&:to_i).each {|ich|
        voltage = status[:HV]-status[:InputDAC][ich]
        system(%Q(root -l -b -q 'fit1.cpp("#{filename}", #{voltage}, #{ich})'))
      } 
    end
  end

  def drawScaler(filename="temp", dac="reg", *ch)
    if dac == "reg" then
      dac = @vmeEasiroc.getRegister("thr")
    else
      dac = dac.to_i
    end

    if ch.empty? then
      64.times {|ich|
        system(%Q(root -l -b -q 'scaler1.cpp("#{filename}", #{dac}, #{ich})'))
      } 
    else
      ch.map(&:to_i).each {|ich|
        system(%Q(root -l -b -q 'scaler1.cpp("#{filename}", #{dac}, #{ich})'))
      } 
    end
  end

  def reset(target)
    if !%w(probe readregister, pedestalsuppression).include?(target)
      puts "unknown argument #{target}"
      return
    end
    
    if target == 'probe'
      @vmeEasiroc.resetProbeRegister
    end
    
    if target == 'readregister'
      @vmeEasiroc.resetReadRegister
    end
    
    if target == 'pedestalsuppression'
      @vmeEasiroc.resetPedestalSuppression
    end
  end

  def timeStamp
    time=Time.now
    puts "Time stamp: #{time}, #{time.to_i}"
  end
  
  def help
  puts <<-EOS
  How to use
  setHV <bias voltage>	input <bias voltage>; 0.00~90.00V to MPPC
  slowcontrol           	transmit SlowControl
  read <EventNum> <FileName>  read <EventNum> events and write to <FileName>
  readloop                    readloop for automatic data taking
  reset probe|readregister    reset setting
  help                        print this message
  version                     print version number
  exit|quit                   quit this program
  
  COMMANDS:
  - adc <on/off>
  - cd <path>
  - sleep <time>
  - exit
  - mode
  - muxControl <ch(0..32)>
  - pwd
  - quit
  - read <EventNum> <FileName>
  - readloop
  - reset <target>
  - scaler <on/off>
  - setHV  <bias voltage (00.00~90.00)>
  - increaseHV <bias voltage>
  - decreaseHV
  - slowcontrol
  - statusInputDAC <ch(0..63) / all(64)>
  - statusHV
  - statusTemp
  - checkHV <voltage_limit=80><current_limit=20><repeat_count=3>
  - setInputDAC <InputDAC voltage (0.0~4.5)>
  - tdc <on/off>
  - version
  - initialCheck
  - read
  - fit
  DIRECT_COMMANDS:
  - cat
  - cp
  - less
  - ls
  - mkdir
  - mv
  - rm
  - rmdir
  - root
  - sleep
  EOS
  end
  
  def version
    versionMajor, versionMinor, versionHotfix, versionPatch,
      year, month, day = @vmeEasiroc.version
    puts "v.#{versionMajor}.#{versionMinor}.#{versionHotfix}-p#{versionPatch}"
    puts "Synthesized on #{year}-#{month}-#{day}"
  end
  
  alias quit exit
end


$logger = Logger.new(STDOUT)
$logger.formatter = proc{|severity, datetime, progname, message|
  "#{message}\n"
}
$logger.level = Logger::INFO
#$logger.level = Logger::DEBUG

OPTS = {}
opt = OptionParser.new
opt.on('-e COMMAND', 'execute COMMAND') {|v| OPTS[:command] = v}
opt.on('-q', 'quit after execute command') {|v| OPTS[:quit] = v}
opt.parse!(ARGV)

#ipaddr = ARGV.shift || "192.168.10.16"
ipaddr = ARGV.shift || "192.168.10.21"

if !ipaddr
  puts "Usage:"
  puts "    #{$0} <Options> <IP Address>"
  exit 1
end

vmeEasiroc = VmeEasiroc.new(ipaddr, 24, 4660)
vmeEasiroc.sendSlowControl
vmeEasiroc.sendProbeRegister
vmeEasiroc.sendReadRegister
vmeEasiroc.sendPedestalSuppression
vmeEasiroc.sendSelectbaleLogic
vmeEasiroc.sendTriggerWidth
vmeEasiroc.sendTimeWindow
vmeEasiroc.sendUsrClkOutRegister
#vmeEasiroc.sendTriggerValues

pathOfThisFile = File.expand_path(File.dirname(__FILE__))
hist = pathOfThisFile + '/hist'

que = Queue.new
commandDispatcher = CommandDispatcher.new(vmeEasiroc, hist,que)

runCommandFile = pathOfThisFile + '/.rc'
begin
  open(runCommandFile) do |f|
    f.each_line do |line|
      commandDispatcher.dispatch(line.chomp)
    end
  end
rescue Errno::ENOENT
end

if OPTS[:command]
  OPTS[:command].split(';').map(&:strip).each do |line|
    commandDispatcher.dispatch(line)
  end
  
  if OPTS[:quit]
    exit
  end
end

def shellCommand
  cache = nil
  proc {
    return cache if cache
    cache = []
    ENV['PATH'].split(File::PATH_SEPARATOR).each do |path|
      if !FileTest::exist?(path)
        next
      end
      
      Dir::foreach(path) do |d|
        if FileTest::executable?(path + '/' + d) &&
          FileTest::file?(path + '/' + d)
          cache << d
        end
      end
    end
    cache.sort!.uniq!
  }
end


shellCommandCompletion = proc {|word|
  comp = shellCommand.call.grep(/\A#{Regexp.quote word}/)
  
  if Readline::FILENAME_COMPLETION_PROC.call(word)
    filenameComp = []
    Readline::FILENAME_COMPLETION_PROC.call(word).each do |file|
      if FileTest::executable?(file) && FileTest::file?(file)
        filenameComp << file
      elsif FileTest::directory?(file)
        filenameComp << file + '/'
      end
    end
    
    if comp.empty? && filenameComp.size == 1 && filenameComp[0][-1] == '/'
      comp = [filenameComp[0] + 'hoge', filenameComp[0] + 'fuga']
    else
      comp.concat(filenameComp)
    end
  end
  comp
}

Readline.completion_proc = proc {|word|
  if word[0] == '!'
    shellCommandCompletion.call(word[1..-1]).map{|i| '!' + i}
  else
    CommandDispatcher::COMMANDS.grep(/\A#{Regexp.quote word}/)
    .concat(Readline::FILENAME_COMPLETION_PROC.call(word) || [])
  end
}

commandHistoryFile = pathOfThisFile + '/.history'
begin
  open(commandHistoryFile) do |f|
    f.each_line do |line|
      Readline::HISTORY << line.chomp
    end
  end
rescue Errno::ENOENT
end

Signal.trap(:INT){
  puts "!!!! Ctrl+C !!!! 'exit|quit' command is recommended."
  puts "Decreasing MPPC bias voltage..."
  commandDispatcher.setHV(0.00)
  sleep 0.2
  puts "Shutdown HV supply..."
  commandDispatcher.shutdownHV
  sleep 0.2
  exit
}
Signal.trap(:TSTP){
  puts "!!!! Ctrl+Z !!!! 'exit|quit' command is recommended."
  puts "Decreasing MPPC bias voltage..."
  commandDispatcher.setHV(0.00)
  sleep 0.2
  puts "Shutdown HV supply..."
  commandDispatcher.shutdownHV
  sleep 0.2
  exit
}
Signal.trap(:TERM){
  puts "!!!! SIGTERM received !!!! Gracefully shutting down..."
  commandDispatcher.setHV(0.00)
  sleep 0.2
  commandDispatcher.shutdownHV
  sleep 0.2
  exit
}

while buf = Readline.readline('> ', true)
  hist = Readline::HISTORY
  if /^\s*$/ =~ buf
    hist.pop
    next
  end
  
  begin
    if hist[hist.length - 2] == buf && hist.length != 1
      hist.pop
    end
  rescue IndexError
  end
  
  if buf[0] == '!'
    if system(buf[1..-1]) == nil
      puts "cannot execute #{buf[1..-1]}"
    end
  else
    begin
      commandDispatcher.dispatch(buf)
    rescue SystemExit
      puts "Decreasing MPPC bias voltage..."
      commandDispatcher.setHV(0.00)
      sleep 0.2
      puts "Shutdown HV supply..."
      commandDispatcher.shutdownHV
      sleep 0.2
      
      exit
    end
  end
  
  begin
    open(commandHistoryFile, 'a') do |f|
      f.puts(buf)
    end
  rescue Errno::EACCES
  end
end
