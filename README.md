# Gravity Simulation
![Planet and moon orbiting a sun](./images/image1.png)
This is an n-body gravity simulation which I created for educational purposes. My aim was to create a tool which can help to develop an intuitive understanding of Newton's law of universal gravitation in a visually interesting way.

## Building 
Use GNU make:

```make libs && make build```

Requires g++.
Tested on linux, but not macOS or windows.

## Dependencies
* SDL2
* OpenGL
* GLM (in submodule)
* ImGUI (in submodule)

## Screenshots
![editing](./images/image2.png)
![3 body](./images/image3.png)
![3 body stable](./images/image4.png)
![3 body relative](./images/image5.png)
![binary star system](./images/image6.png)
![binary chaotic](./images/image7.png)
![solar system](./images/image8.png)