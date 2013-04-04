
class ResizeFilterProto(object):
    """ Resize filter prototype """

    def __init__(self, input=None, output=None, width=None, height=None):
        self.input = input
        self.output = output
        self.width = width
        self.height = height
    
    def resize(self, input=None, output=None, width=None, height=None):
        if input is None and self.input is None:
            raise StandardError('No input supplied')
        
        if width is None and self.width is None:
            raise StandardError('No width supplied')
        
        if height is None and self.height is None:
            raise StandardError('No height supplied')

class ResizeFilterNearest(ResizeFilterProto):
    def resize(self, input=None, output=None, width=None, height=None):
        super(self.__class__, self).resize( input=input,
                                            output=output,
                                            width=width,
                                            height=height)
        
        if width  is None:  width = self.width
        if height is None:  height = self.height
        if input  is None:  input = self.input
        if output is None:  output = self.output
        
        if output is None:
            #
            # FIXME: Create buffer to use
            #
            raise StandardError('Not Implemented')
        
        

class ResizeFilterBilinear(ResizeFilterProto): pass
class ResizeFilterCubic(ResizeFilterProto): pass
