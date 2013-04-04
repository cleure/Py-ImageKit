import math

class Conversion(object):
    @staticmethod
    def convert_to(values):
        pass
    
    @staticmethod
    def convert_from(values):
        pass

class RGB_HSV(Conversion):
    @staticmethod
    def convert_to(values):
        """ RGB to HSV (scale of 0.0 to 1.0) """
        
        r, g, b = values[0]/1.0, values[1]/1.0, values[2]/1.0
        
        mx = max(r, g, b)
        mn = min(r, g, b)
        df = mx-mn
        
        if   mx == mn: h = 0.0
        elif mx == r:  h = (60.0 * ((g-b)/df) + 360) % 360
        elif mx == g:  h = (60.0 * ((b-r)/df) + 120) % 360
        elif mx == b:  h = (60.0 * ((r-g)/df) + 240) % 360
        
        if mx == 0:
            s = 0.0
        else:
            s = df/mx
            
        h = float(h) * (1.0/360)
        s = float(s)
        v = float(mx)
        
        if len(values) > 3:
            return h, s, v, a
        
        return h, s, v

    @staticmethod
    def convert_from(values):
        """ HSV to RGB (scale of 0.0 to 1.0) """
    
        h = round(values[0] * (360/1.0), 6)    # Avoid rounding error
        s = values[1]
        v = values[2]
        
        h60 = h / 60.0
        h60f = math.floor(h60)
        hi = int(h60f) % 6
        
        f = h60 - h60f
        p = v * (1 - s)
        q = v * (1 - f * s)
        t = v * (1 - (1 - f) * s)
        
        r, g, b = 0, 0, 0
        if   hi == 0: r, g, b = v, t, p
        elif hi == 1: r, g, b = q, v, p
        elif hi == 2: r, g, b = p, v, t
        elif hi == 3: r, g, b = p, q, v
        elif hi == 4: r, g, b = t, p, v
        elif hi == 5: r, g, b = v, p, q
        
        return r*1.0, g*1.0, b*1.0

def main():
    si = RGB_HSV.convert_to([0.0, 1.0, 0.5])
    so = RGB_HSV.convert_from(si)
    print si
    print so
    print ''
    

if __name__ == '__main__':
    main()
