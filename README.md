# unreal_sfotbody
This is a unreal c++ actor class that contains some primitive soft body meshes. It works on the spring-mass model with internal springs, and simple Euler integration every timestep. <br> 
Point masses are present at all the vertices in the meshes. surphase springs are present between all edges, and internal springs connect a point at the center to all vertices <br>
Only ground collisions are supported for now <br>
<br>
Parameters SPRING_K(spring constant) and SPRING_DAMP(damping constant) can be tweaked to obtain different behaviours of the spring. They should be scaled, however, according to the PM_MASS(mass of each of the point masses). High or low value of any of the constants defined at the top of SoftBodyACtor.cpp will cause the simulation to break.

https://user-images.githubusercontent.com/66353349/203005452-2458b673-5d86-45a7-a030-09c6d752a795.mp4



https://user-images.githubusercontent.com/66353349/203005467-ce82e0ca-c730-4b75-9d01-b98e06c53811.mp4



https://user-images.githubusercontent.com/66353349/203005479-85aa3baf-08b5-4f45-b491-59a53f61bea3.mp4

