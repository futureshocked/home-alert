require 'sinatra'
configure do
    require 'redis'    
    configure(:production) do    	
		    uri = URI.parse(ENV["REDISCLOUD_URL"])
		    $redis = Redis.new(:host => uri.host, :port => uri.port, :password => uri.password)
     	end
    configure(:development){ $redis = Redis.new } 
    
    set :server, :puma
end


get '/' do
	erb :index	
end

post '/post_message' do
	require "json"
	erb :post_message	
	data = { params[:element_1] => {}}
	$redis.set( params[:element_1], { "message" => params[:element_2], 
									 "buzzer" => params[:element_3_1]}.to_json)

	# redis.set("dmd_id", params[:element_1])
	# $redis.set("message", params[:element_2])
	# redis.set("buzzer", params[:buzzer])

	"Thank you, message posted."
end

get '/get_message/:dmd_id' do	
	require "json"
	response = JSON.parse($redis.get(params[:dmd_id]))
	if response["buzzer"] 
		response["message"] + "\n1\n"
	else
		response["message"] + "\n0\n"
	end
end

