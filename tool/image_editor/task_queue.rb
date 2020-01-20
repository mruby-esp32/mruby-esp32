class TaskQueue
  def self.init
    Thread.abort_on_exception = true
    @thread_queue = []
  end

  def self.push(task, log: nil, mode: nil)
    @thread_queue << Thread.new do
      begin
        task.call(mode: mode)
      rescue StandardError => e
        if log
          log.puts '---ERROR in a thread---'
          log.puts e.to_s
          e.backtrace.each { |t| log.puts t }
        end
      end
    end
  end

  def self.join
    @thread_queue.each(&:join)
  end
end

