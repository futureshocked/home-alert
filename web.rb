require 'sinatra'
configure do
    require 'redis'    
    configure(:production){  
    uri = URI.parse(ENV["REDISCLOUD_URL"])
    $redis = Redis.new(:host => uri.host, :port => uri.port, :password => uri.password)
     }
    configure(:development){ $redis = Redis.new } 
    
    set :server, :puma
end


get '/' do
	erb :index
	# redis.set("mykey", "hello world2")
	# redis.get("mykey")
  # "Hello, world\n" 
end

post '/post_message' do
	# redis = Redis.new
	# redis.set("dmd_id", params[:element_1])
	$redis.set("message", params[:element_2])
	# redis.set("buzzer", params[:buzzer])

	"Thank you, message posted."
end

get '/get_message' do
	# redis = Redis.new
	$redis.get("message") + "\n"
end

