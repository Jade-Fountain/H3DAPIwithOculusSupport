
from H3DInterface import *
 
# make sure the proxy is drawn at the position of the haptics device
tex_channels = 1
tex_size_s = 32
tex_size_t = 32
tex_matrix = []
for i in range( tex_size_s * tex_size_t ):
  tex_matrix.append( "0xFF" )
 
appearance_node, = references.getValue()
 
string_of_values = ""
for i in tex_matrix:
  string_of_values= string_of_values + i + " "
pixelTexture =  createX3DNodeFromString( """<PixelTexture repeatS="false" repeatT="false" image=" %i %i %i %s"  />
""" % (tex_size_s, tex_size_t, tex_channels, string_of_values) )[0]
appearance_node.texture.setValue( pixelTexture )
 
class DrawTexCoord( AutoUpdate( MFVec3f ) ):
  def update( self, event ):
    event_value = event.getValue()
    for value in event_value:
      # added because 0,0,0 is sent as first event, maybe due to resize.
      if value.x == value.y == value.z == 0:
        return event_value
      s_index = int(value.x * tex_size_s)
      if s_index >= tex_size_s:
        s_index = tex_size_s - 1
      t_index = int((1 - value.y) * tex_size_t)
      if t_index >= tex_size_t:
        t_index = tex_size_t - 1
      if( tex_matrix[t_index * tex_size_t + s_index] != "0x50" ):
        tex_matrix[t_index * tex_size_t + s_index] = "0x50"
        string_of_values = ""
        for i in tex_matrix:
          string_of_values= string_of_values + i + " "
        pixelTexture =  createX3DNodeFromString( """<PixelTexture repeatS="false" repeatT="false" image=" %i %i %i %s"  />
        """ % (tex_size_s, tex_size_t, tex_channels, string_of_values) )[0]
        appearance_node.texture.setValue( pixelTexture )
    return event_value
 
drawTexCoord = DrawTexCoord()
 
 