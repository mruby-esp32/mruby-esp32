require 'tk'
require_relative './task_queue'

class RGB222
    attr_accessor :r,:g,:b,:a
    def initialize(r,g,b,a=0)
        @r = r
        @g = g
        @b = b
        @a = a
    end

    def is_a?
        return true if @a>0
        false
    end

    def self.bit(x)
        (x * 5).to_s(16)
    end

    def bit(x)
        (x * 5).to_s(16)
    end

    def to_s
        "\##{bit(@r)}#{bit(@g)}#{bit(@b)}"
    end

    def self.color(r,g,b)
        "\##{bit(r)}#{bit(g)}#{bit(b)}"
    end
end

class ImageEditorApp
    AppVersion = "0.1.0"
    def initialize
        @cv = nil #Canvas
        @pix_unit_w = 16
        @pix_unit_h = 16
        @vscale = 32
        @cw = @pix_unit_w*@vscale
        @ch = @pix_unit_h*@vscale

        @picdata = Array.new(@pix_unit_w).map{Array.new(@pix_unit_h){RGB222.new(3,3,3)}}

        @current_color = RGB222.new(0,0,0)
    end

    def start_app
        # UI setting
        Tk.root.title('Image Editor')
        f0 = TkFrame.new
        f1 = TkFrame.new(f0)
        f2 = TkFrame.new(f0)
        f21 = TkFrame.new(f2)
        f22 = TkFrame.new(f2)

        #=======================================
        # File selection box
        file_select = TkVariable.new('./')
        entry_file = TkEntry.new(f1, width: 80, textvariable: file_select)

        # File selection button
        proc_open = proc do
            filetype = '{{All Files} {*}}'
            file_select.value = Tk.getOpenFile(filetypes: filetype) # , initialdir: $path_name
            puts "Load start"
        end
        btn_select = TkButton.new(f1, text: 'Load', command: proc_open)

        proc_save = proc do
            save_data(file_select.value)
        end
        btn_save = TkButton.new(f1, text: 'Save', command: proc_save)

        entry_file.pack(padx: 10, side: 'left')
        btn_select.pack(padx: 10,side: 'left')
        btn_save.pack(side: 'left')

        #=======================================
        #lb2 = TkLabel.new(f2, text: 'f2')
        #lb2.pack(side: 'top')

        #=======================================
        lb21 = TkLabel.new(f21, text: 'Edit Area')
        lb21.pack(side: 'top')

        canvas = TkCanvas.new(f21, width: @cw, height: @ch, background: 'white')
        @cv = canvas
        update_canvas
        canvas.bind('Button', proc{|x,y,b|click_on_canvas(x,y,b)}, '%x','%y','%b')

        #=======================================
        lb22 = TkLabel.new(f22, text: 'Pallet Area')
        lb22.pack(side: 'top')

        pcanvas = TkCanvas.new(f22, width: 100, height: 300, background: 'white')
        @pallet = pcanvas
        pcanvas.bind('Button', proc{|x,y,b|click_on_pallet(x,y,b)}, '%x','%y','%b')
        draw_pallet

        #---------------------------------------
        canvas.pack()
        pcanvas.pack()

        f0.pack
        f1.pack(side: 'top')
        f2.pack(side: 'top')
        f21.pack(side: 'left')
        f22.pack(side: 'left')
        Tk.mainloop
    end

    def draw_px(x,y,color)
        if color.is_a?
            TkcRectangle.new(@cv, 
                x*@vscale          , y*@vscale, 
                x*@vscale+@vscale-1, y*@vscale+@vscale-1 ) do
                outline("")
                fill("#BBB")
            end
            TkcLine.new(@cv, 
                x*@vscale          , y*@vscale, 
                x*@vscale+@vscale-1, y*@vscale+@vscale-1 ) do
                fill("#F00")
            end
            return
        end
        TkcRectangle.new(@cv, 
            x*@vscale          , y*@vscale, 
            x*@vscale+@vscale-1, y*@vscale+@vscale-1 ) do
            outline("")
            fill(color.to_s)
        end
    end

    def update_px(x,y)
        draw_px(x,y,@picdata[x][y])
    end

    def update_canvas
        (0...@pix_unit_w).each do |x|
            (0...@pix_unit_h).each do |y|
                #puts "#{x},#{y}"
                update_px(x,y)
            end
        end
    end

    def update_current_color
        col =  @current_color.to_s
        TkcRectangle.new(@pallet, 
            1      , 1, 
            1+98   , 1+@top_mergin-2 ) do
            outline("")
            fill(col)
        end
    end

    def draw_pallet
        @top_mergin = 40

        @bwidth  = 23
        @xmergin = 25

        @bheight = 14
        @ymergin = 16

        draw_box = Proc.new do |x,y,color|
            TkcRectangle.new(@pallet, 
                x*@xmergin           , @top_mergin+y*@ymergin, 
                x*@xmergin+@bwidth   , @top_mergin+y*@ymergin+@bheight ) do
                outline("")
                fill(color)
            end
        end

        (0..3).each do |x|
            (0..15).each do |y|
                #puts "#{x},#{y}"
                r = x
                g = y / 4
                b = y % 4
                draw_box.call(x,y,RGB222.color(r,g,b))
            end
        end
        update_current_color
    end

    def click_on_canvas(x,y,bn)
        bx = x / @vscale
        by = y / @vscale
        return if bx < 0
        return if bx >= @pix_unit_w
        return if by < 0
        return if by >= @pix_unit_h
        
        #puts"1 #{x},#{y},#{bn} => #{bx},#{by}"
        case bn
        when 1
            @picdata[bx][by] = @current_color
        when 2
            @picdata[bx][by] = RGB222.new(0,0,0,1)
        end
        update_px(bx,by)
    end


    def click_on_pallet(x,y,bn)
        #puts"2 #{x},#{y},#{bn}"
        ty = y-@top_mergin
        dx = x / @xmergin
        dy = ty / @ymergin
        #puts"2a #{dx},#{dy}"
        return if dx < 0 or dx >= 4
        return if dy < 0 or dy >= 16
        @current_color = RGB222.new(dx,dy/4,dy%4)
        update_current_color
    end

    def save_data(path)
        puts "Save #{path}"
        
        header = 0xFFFFFF11

        File.open(path,"wb") do |fp|
            fp.write([ header ].pack("V")) #pack uint32_t
            fp.write([ @pix_unit_w ].pack("v")) #pack uint16_t little:v  big:n
            fp.write([ @pix_unit_h ].pack("v"))
        
            (0...@pix_unit_h).each do |y| 
                (0...@pix_unit_w).each do |x|
                    r = @picdata[x][y].r
                    g = @picdata[x][y].g
                    b = @picdata[x][y].b
                    a = @picdata[x][y].a
        
                    #val = (b << 4) | (g << 2) | r
                    val = (b << 6) | (g << 4) | (r << 2) | a
                    p val
                    fp.write([val].pack("C"))
                end 
            end        
        end
        puts "Save done"
    end

end
    
$log = nil
begin
    TaskQueue.init
    ImageEditorApp.new.start_app
rescue StandardError => e
  puts '.---ERROR---'
  p e
  e.backtrace.each { |t| puts t }
ensure
  TaskQueue.join
  #$log.close if $log
  puts 'end'
end
