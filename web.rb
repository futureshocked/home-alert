require 'sinatra'
# Run like this: "foreman start", using the Heroku Toolbelt
configure do
    require 'redis'    
    configure(:production) do    	
		    uri = URI.parse(ENV["REDISCLOUD_URL"])
		    $redis = Redis.new( :host => uri.host, 
		    					:port => uri.port, 
		    					:password => uri.password)
     	end
    configure(:development){ $redis = Redis.new } 
    
    set :server, :puma
end


get '/' do
	erb :index	
end

post '/post_message' do
	require "json"
	# erb :post_message		
	$redis.set( params[:element_1], { "message" => params[:element_2], 
									 "buzzer" => params[:element_3_1]}.to_json)
	"Thank you, message posted."
end

get '/get_message/:dmd_id' do	
	require "json"
	response = JSON.parse($redis.get(params[:dmd_id]))
	if response["buzzer"] 
		"1" + response["message"] + "\n"
	else
		"0" + response["message"] + "\n"
	end
end

get '/cricket_match_description/:match_id' do
	require 'json'
	require 'mechanize'
	require 'open-uri'
	a = Mechanize.new
	a.get("http://cricscore-api.appspot.com/csa?id=#{params[:match_id]}")
	scores = a.page.body
	m_s = JSON.parse(scores)
	m_s[0]["de"]
end

get '/cricket_match_list' do
	require 'json'
	require 'mechanize'
	require 'open-uri'
	a = Mechanize.new
	a.get("http://cricscore-api.appspot.com/csa")
	j = a.page.body
	b = JSON.parse(j)
	response = ""
	b.each { |r| response += r["id"].to_s + " : " + r["t2"] + " vs " + r["t1"] + "\n"}
	puts response
	response
end
