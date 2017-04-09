/**
 * The Processing-Syphon library allows to create Syphon clients
 * and servers in a Processing sketch to share frames with other
 * applications. It only works on MacOSX and requires the P3D
 * renderer.
 *
 * (c) 2011-15
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 * 
 * @author		Andres Colubri http://andrescolubri.net/
 * @modified	09/13/2015
 * @version		##version##
 */

package codeanticode.syphon;

import processing.core.*;
import processing.opengl.*;
import jsyphon.*;

/**
 * Syphon server class. It broadcasts the textures encapsulated in 
 * PImage objects.
 *
 */
public class SyphonServer {
	protected PApplet parent;
	protected PGraphicsOpenGL pg;
	protected JSyphonServer server;
	protected String serverName;
	
  /**
   * Constructor that sets server with specified name.
   * 
   * @param parent
   * @param serverName
   */	
  public SyphonServer(PApplet parent, String name) {
    this.parent = parent;
    pg = (PGraphicsOpenGL)parent.g;
    serverName = name;
    Syphon.init();
    parent.registerMethod("dispose", this);
  }
  
  
  /**
   * This method is called automatically when the sketch is disposed, so making
   * sure that the server is properly stopped and Syphon doesn't complain about
   * memory not being properly released:
   * https://github.com/Syphon/Processing/issues/4
   * 
   */
  public void dispose() {
    server.stop();
  } 
	
  
  /**
   * The class finalizer tries to make sure that the server is stopped. No 
   * guarantee that this method is ever called by the GC, but just in case.
   * 
   */
  protected void finalize() throws Throwable {
    try {
      if (server != null) {
        server.stop();
      }
    } finally {
      super.finalize();
    }
  }  
  
  
  /**
   * Returns true if this server is bound to any
   * client.
   * 
   * @return boolean 
   */    
  public boolean hasClients() {
    return server.hasClients();
  }	
	
  
  /**
   * Sends the source image to the bound client.
   * 
   * @param source
   */ 	
  public void sendImage(PImage source) {
    if (parent.frameCount == 0) {
      PGraphics.showWarning("Only can send frames in draw()");
      return;
    }
    
    Texture tex = pg.getTexture(source);
    if (tex != null) {
      if (server == null) {
        // The server needs to be created after setup and all the 
        // JOGL initialization has taken place. Otherwise frame
        // sending doesn't work...
        server = new JSyphonServer();
        server.initWithName(serverName);        
      }
      
      server.publishFrameTexture(tex.glName, tex.glTarget, 
                                 0, 0, tex.glWidth, tex.glHeight, 
                                 tex.glWidth, tex.glHeight, false);
    } else {
      PGraphics.showWarning("Texture is null");
    }
  }	
  
  
  /**
   * Sends the screen image to the bound client.
   * 
   */   
  public void sendScreen() {
    pg.pgl.requestFBOLayer();
    sendImage(pg);    
  }
  
  
  /**
   * Stops the server.
   * 
   */  
  public void stop() {
    server.stop();
  }
}

