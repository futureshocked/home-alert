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

