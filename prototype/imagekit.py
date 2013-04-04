

"""

TODO: Implement this class in C

"""

class ImageBuffer(object):
    def __init__(self, width, height, data=[], pitch=None, channels=3):
        if pitch is None:
            pitch = width
        
        self.channels = channels
        self.width = width
        self.height = height
        self.pitch = pitch
        self.data = []
        
        if len(data) == 0:
            self.data = [[0.0, 0.0, 0.0] for i in range(pitch * height)]
        else:
            self.data = data
    
    def get_pixel_idx(self, x, y):
        return (y * self.pitch) + x
    
    def get_pixel(self, x, y):
        return self.data[self.get_pixel_idx(x, y)]
    
    def set_pixel(self, x, y, rgb):
        self.data[self.get_pixel_idx(x, y)] = [rgb[i] for i in xrange(self.channels)]
    
    def get_box(self, center_x, center_y, size=3):
        """
        Returns a matrix of pixels of (size)x(size), with pixels from starting point
        (center_x, center_y). If pixel is out of range, None is used in place of a
        list of RGB values.
        """
        pass

def scale_nearest(self, input, output, width, height):
    y2_mult = int((input.height << 16) / height + 1)
    x2_mult = int((input.width << 16) / width + 1)
    
    for y in range(height):
        y2 = (y * y2_mult) >> 16
        for x in range(width):
            x2 = (x * x2_mult) >> 16
            output.set_channel(x, y, 0, input.get_channel(x2, y2, 0))
            output.set_channel(x, y, 1, input.get_channel(x2, y2, 1))
            output.set_channel(x, y, 2, input.get_channel(x2, y2, 2))
        
    return output
