# SPDX-License-Identifier: GPL-3.0
class Database
  #----------------------------------------------------------------------------
  def self.initialize_for(name)
    db_file = File.join(File.dirname(__FILE__), "..", "#{name}.db")
    db_file_new = File.exists?(db_file)

    ActiveRecord::Base.establish_connection({
      :adapter => "sqlite3",
      :database => db_file
    })

    # With huge data, this should reduce the offset a little.
    ActiveRecord::Base.connection.execute("PRAGMA page_size = 4096;")
    # If ON, writing on an average HDD takes tons of time.
    ActiveRecord::Base.connection.execute("PRAGMA synchronous = OFF;")

    unless db_file_new
      create_tables
    end
  end

  #----------------------------------------------------------------------------
  def self.setup_keys
    models.each do |model|
      if model.respond_to?(:add_table_keys)
        ActiveRecord::Schema.define { |b| model.add_table_keys(b) }
      end
    end
  end

  private

  #----------------------------------------------------------------------------
  def self.create_tables
    models.each do |model|
      if model.respond_to?(:create_table)
        ActiveRecord::Schema.define { |b| model.create_table(b) }
      end
    end
  end

  #----------------------------------------------------------------------------
  def self.models
    ObjectSpace.each_object(Class).select { |klass| klass < ActiveRecord::Base }
  end
end
