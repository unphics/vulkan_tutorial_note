
/*由顶点着色器的位置形成的三角形用片段填充屏幕上的一个区域。在这些片段上调用片段着色器以生产帧缓冲区的颜色和深度。*/

#version 450
//输入变量不一定必须使用相同的名称，它们将使用location指令指定的索引链接在一起。该面函数已经修改为输出颜色和Aloha值
//fragColor将自动为三个定点之间的片元插值，从而产生平滑的渐变
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
/*该main函数被每个片元调用，就像顶点着色器mian函数被每个顶点调用一样。glsl中的颜色是4分量向量，其rgpa通道在[0,1]范围内。
与gl_Position顶点着色器不同，没有内置变量来为当前片元输出颜色。必须为每个帧缓冲区指定自己的输出变量，其中layout(location=0)修饰符指定帧缓冲区的索引。
颜色被写入此outColor变量，改变量链接到index处的第一个(也是唯一一个)*/