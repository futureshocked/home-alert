
require 'json'
require 'mechanize'
require 'open-uri'
a=Mechanize.new
a.get("http://cricscore-api.appspot.com/csa")
j = a.page.body
b=JSON.parse(j)
b.length
 b[0]
match = b[1]["id"]  # 783877 
a.get("http://cricscore-api.appspot.com/csa?id=#{match}")
scores = a.page.body
m_s = JSON.parse(scores)
m_s[0]["de"]