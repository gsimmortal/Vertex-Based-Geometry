#version 330

in vec4 colour;
in vec4 newPos;
in float colProfile;
in float animate;

layout (location=0) out vec4 fragColour;

void main(void) {

    //vec4 newNewPos = normalize(newPos);
    if (colProfile == 0.0)
    {
        fragColour = vec4(2.0*newPos.y,0.0,(2.0-(2.0*newPos.y)),1.0);
    }
    else if (colProfile >= 0.1-0.01 && colProfile <= 0.1+0.01)
    {
        float tmpR = sin(newPos.y+1);
        float tmpG = cos(newPos.y+1);
        float tmpB = tan(newPos.y+1);

        vec4 newColour = vec4(tmpR,tmpG,tmpB,1.0);
        fragColour = newColour;

    }
    else if (colProfile >= 0.2-0.01 && colProfile <= 0.2+0.01)
    {
        float tmpR = sin(newPos.y+1);
        float tmpG = cos(newPos.y+1);
        float tmpB = -sin(newPos.y+1);

        vec4 newColour = vec4(tmpR,tmpG,tmpB,1.0);
        fragColour = newColour;

    }
    else if (colProfile >= 0.3-0.01 && colProfile <= 0.3+0.01)
    {
        float tmpR = tan(newPos.x+(animate/3.6));
        float tmpG = tan(newPos.z-(animate/3.6));
        float tmpB = cos(newPos.y-0.5);

        vec4 newColour = vec4(tmpR,tmpG,tmpB,1.0);
        fragColour = newColour;

    }
    else if (colProfile >= 0.4-0.01 && colProfile <= 0.4+0.01)
    {
       float tmpR = sin(newPos.y);//+(animate/0.36);
       float tmpG = asin(newPos.y);//+(animate/3.6);
       float tmpB = cos(newPos.y);//+(animate/36);

       vec4 newColour = vec4(tmpR,tmpG,tmpB,1.0);
       fragColour = newColour;

    }
     else if (colProfile >= 0.5-0.01 && colProfile <= 0.5+0.01)
    {
        float tmpR = sqrt(exp(newPos.y)) / 2;
        float tmpG = newPos.y / 15;
        float tmpB =  inversesqrt(exp(newPos.y)) / 50;

        vec4 newColour = vec4(tmpR,tmpG,tmpB,1.0);
        fragColour = newColour;

    }

    if (mod(newPos.x,0.5) <= 0.06 || mod(newPos.z,0.5) <= 0.06)fragColour = vec4(0.0,0.0,0.0,1.0);

}
