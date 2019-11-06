#version 330

varying  vec4 color;
varying vec3 vpos;
varying vec2 texcoord;
varying vec3 normal;

uniform vec3 eye_position;
uniform vec3 light_position;

uniform int type;


//phong lighting model

//l is norm(vpos - light_position)
//v is norm(vpos - eye_position)
//r comes from l and normal, using reflect()
//n is the normal that's been passed in


void main()
{
  gl_FragColor = color;
  // gl_FragColor = vec4(texcoord.xy,0,1);

  vec3 l = normalize(vpos - light_position);
  vec3 v = normalize(vpos - eye_position);
  vec3 n = normalize(normal);
  vec3 r = normalize(reflect(l, n));



    float a = 0.08;

    float d = (1/(pow(0.25*distance(vpos,light_position),2))) * 0.3 * max(dot(n, l),-0.4);

    float s = (1/(pow(0.25*distance(vpos,light_position),2))) * 1.0 * pow(max(dot(r,v),0),100);


    if(type == 0 && gl_FrontFacing)
    {
      gl_FragColor.xyz += a*vec3(0.1,0.1,0.2);
      gl_FragColor.xyz += d*vec3(0.2,0.2,0.16);
      if(dot(n,l) > 0)
        gl_FragColor.xyz += s*vec3(1,1,0);
    }

    if(type == 1 && gl_FrontFacing)
    {
      gl_FragColor.xyz += a*vec3(0.6,0.1,0.2);
      gl_FragColor.xyz += d*vec3(0.5,0.2,0.16);
      if(dot(n,l) > 0)
      {
        s = (1/(pow(0.25*distance(vpos,light_position),2))) * 1.2 * pow(max(dot(r,v),0),1000);

        gl_FragColor.xyz += s*vec3(0.3,0.5,0);
      }
    }

    if(type == 0 && !gl_FrontFacing)
    {
      gl_FragColor = gl_FragColor.rgra;

      gl_FragColor = vec4(0.3,0.17,0.05,1);

      float a = 0.1;
      gl_FragColor.xyz += a*vec3(0,0.1,0.16);

      n = -n;
      float d = (1/(pow(0.25*distance(vpos,light_position),2))) * 0.68 * max(dot(n, l),0);
      gl_FragColor.xyz += d*vec3(0.22,0.22,0);

      r = normalize(reflect(l,-n));
      float s = (1/(pow(0.25*distance(vpos,light_position),2))) * 0.88 * pow(max(dot(r,v),0),3);


      //apply specular to the front face only
      if(dot(n,l) > 0)
        gl_FragColor.xyz += s*vec3(vec2(0.35),0);


      if(int(gl_FragCoord.y) % 3 == 0)
      {
        gl_FragColor = vec4(vec3(0.2),1);
      }

    }

    if(type == 1 && !gl_FrontFacing)
    {
      discard;
    }

    if(type == 2 && !gl_FrontFacing || type == 2 && gl_FrontFacing)
    {
      // gl_FragColor = vec4(0.9,0.17,0.05,1);
      gl_FragColor = gl_FragColor.rgra;

      gl_FragColor = vec4(0.1,0.17,0.05,1);

      float a = 0.1;
      gl_FragColor.xyz += a*vec3(0,0.1,0.16);

      n = -n;
      float d = (1/(pow(0.25*distance(vpos,light_position),2))) * 1.28 * max(dot(n, l),0);
      gl_FragColor.xyz += d*vec3(0.22,0.22,0);

      r = normalize(reflect(l,-n));
      float s = (1/(pow(0.25*distance(vpos,light_position),2))) * 1.28 * pow(max(dot(r,v),0),3);


      //apply specular to the front face only
      if(dot(n,l) > 0)
        gl_FragColor.xyz += s*vec3(vec2(0.35),0);

    }

    // if(type == 2 && gl_FrontFacing)
    // {
    //   // gl_FragColor = vec4(0.9,0.17,0.05,1);
    // }


  // int fcxmod = int(gl_FragCoord.x) % 2;
  // int fcymod = int(gl_FragCoord.y) % 2;
  //
  // if((fcymod == 0) || (fcxmod == 0))
  // {
  //   discard;
  // }


//depth coloring
  gl_FragColor.xyz *= 0.2*(1/gl_FragCoord.z);

  // gl_FragColor.y *=  noise1(gl_FragCoord.z);
  // gl_FragColor.y *=  0;
}
