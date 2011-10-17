require 'rubygems'
gem 'mysql'
require 'mysql'
gem 'fastercsv'
require 'fastercsv'

db = Mysql.new('localhost', 'root', 'NoHuHevVig', 'mythconverg')

db.query("truncate table dtv_multiplex")
db.query("truncate table channel")

FasterCSV.foreach("/home/peter/slowlane/output") do |row|
  rs = db.query("select * from dtv_multiplex where transportid = #{row[0].to_i}")

  if (rs.num_rows == 0)
    db.query("INSERT INTO dtv_multiplex (sourceid, transportid, networkid, frequency, symbolrate, polarity, mod_sys, hierarchy, modulation, constellation) VALUES (1, #{row[0].to_i}, #{row[1].to_i}, #{row[2].to_i * 10}, #{row[3].to_i * 100}, '#{(row[4].to_i == 0 ? 'h' : 'v')}', '#{row[5].to_i == 0 ? 'DVB-S' : 'DVB-S2'}', 'a', 'qpsk', 'qpsk')")

    mplexid = db.insert_id
  else
    drs = rs.fetch_row
    mplexid = drs[0]
  end 

  name = db.escape_string(row[9])
  db.query("INSERT INTO channel (chanid, channum, sourceid, callsign, name, useonairguide, mplexid, serviceid) VALUES (#{row[8].to_i}, #{row[8].to_i}, 1, '#{name}', '#{name}', 0, #{mplexid}, #{row[7].to_i})")


end
