require 'sinatra'
configure { set :server, :puma }

get '/' do
  "Hello, world"
  "\n"
end