
require 'lumino'
include Lumino

#EngineSettings.set_engine_log_enabled(true)
#Engine.initialize

class App < Application
    def on_initialize()
        p "!!!!!!!!!!!! init"
    end
end

Lumino::Application.run_app_internal(App.new)

#texture1 = Texture2D.load("logo.png")
#sprite1 = Sprite.new(texture1, 2, 2)

#while Engine.update do

#end
