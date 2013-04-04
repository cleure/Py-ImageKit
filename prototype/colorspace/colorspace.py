
COLORSPACE_SCALE_RGB_15     = ((31,  31,  31),  (0, 0, 0))
COLORSPACE_SCALE_RGB_16     = ((31,  63,  31),  (0, 0, 0))
COLORSPACE_SCALE_RGB_24     = ((255, 255, 255), (0, 0, 0))
COLORSPACE_SCALE_HSV        = ((359,   1,   1), (0, 0, 0))

class Colorspace(object):
    min_channels = 0
    max_channels = 0
    
    def __init__(self, _max, _min=None):
        self._scale_in = [0 for i in range(len(_max))]
        self._scale_out = [0 for i in range(len(_max))]
        
        if _min is None:
            _min = [0 for i in range(len(_max))]
        
        for i in range(len(_max)):
            self._scale_in[i]   = 1.0 / (abs(_min[i]) + _max[i])
            self._scale_out[i]  = (abs(_min[i]) + _max[i]) / 1.0
        
        self.min = _min
        self.max = _max
        self.channels = len(_max)
        
        if self.channels < self.min_channels:
            raise StandardError(
                '%s must have at least %d channels' %
                (self.__class__.__name__, self.min_channels))
        
        if self.channels > self.max_channels:
            raise StandardError(
                '%s must have no greater than %d channels' %
                (self.__class__.__name__, self.max_channels))
    
    def scale_in(self, values):
        return [    self._scale_in[i]   * (values[i] - self.min[i])
                    for i in range(self.channels)]
    
    def scale_out(self, values):
        return [    (self._scale_out[i] * values[i]) + self.min[i]
                    for i in range(self.channels)]

class RGB(Colorspace):
    min_channels = 3
    max_channels = 4

class HSV(Colorspace):
    min_channels = 3
    max_channels = 4

class Grayscale(Colorspace):
    min_channels = 1
    max_channels = 2

def main():
    rgb = RGB(COLORSPACE_SCALE_RGB_16[0], COLORSPACE_SCALE_RGB_16[1])
    si = rgb.scale_in([31, 45, 29])
    so = rgb.scale_out(si)
    
    print si
    print so

if __name__ == '__main__':
    main()
