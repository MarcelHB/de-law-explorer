# SPDX-License-Identifier: GPL-3.0
require File.join(File.dirname(__FILE__), "loader.rb")

Database.initialize_for("test")

ActiveRecord::Base.connection.execute("DELETE FROM inter_code_links")

Code.find_in_batches do |batch|
  batch.each do |code|
    puts "#{code.id}"
    links = Code.select("co.id AS co_id, COUNT(co.id) AS count")
            .joins([
              "INNER JOIN code_atoms cai ON cai.code_id = codes.id",
              "INNER JOIN code_links cl ON cl.code_atom_id = cai.id",
              "INNER JOIN code_atoms cao ON cao.id = cl.to_code_atom_id",
              "INNER JOIN codes co ON cao.code_id = co.id "
            ])
            .where("codes.id = #{code.id} AND codes.id <> co.id")
            .group("co.id")

    links.each do |link|
      InterCodeLink.create({
        :code_id => code.id,
        :to_code_id => link.attributes["co_id"],
        :count => link.attributes["count"]
      })
    end
  end
end

