/* ===================================================
 *  file:       odrSpiral.c
 * ---------------------------------------------------
 *  purpose:	free sample for computing spirals
 *              in OpenDRIVE applications //这行注释说明了这段代码的用途：它是一个用于在OpenDRIVE应用程序中计算螺旋线的示例代码。
 * ---------------------------------------------------
 *  first edit:	09.03.2010 by M. Dupuis @ VIRES GmbH
 *  last mod.:  09.03.2010 by M. Dupuis @ VIRES GmbH
 * ===================================================
 
    Copyright 2010 VIRES Simulationstechnologie GmbH//这表明这段代码的版权归VIRES Simulationstechnologie GmbH所有，版权年份是2010年。

    Licensed under the Apache License, Version 2.0 (the "License");//这表示代码是在Apache License 2.0版本下授权的。Apache License是一种开源软件许可证，允许用户自由使用、修改和分发软件。
    you may not use this file except in compliance with the License.//这意味着用户必须遵守Apache License 2.0才能使用这段代码。
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0//提供了获取许可证副本的链接：http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.//这是典型的开源软件免责声明，表明软件是“按原样”提供的，没有任何形式的保证。
    See the License for the specific language governing permissions and
    limitations under the License.//这提示用户查看许可证以了解具体的权限和限制。
 */
 
/**
* compute the actual "standard" spiral, starting with curvature 0
* @param s      run-length along spiral//这表示参数s是沿着螺旋线的运行长度。
* @param cDot   first derivative of curvature [1/m2]//参数cDot是曲率的一阶导数，单位是每平方米（1/m²）。
* @param x      resulting x-coordinate in spirals local co-ordinate system [m]//参数x是在螺旋线局部坐标系中的x坐标，单位是米（m）。
* @param y      resulting y-coordinate in spirals local co-ordinate system [m]//参数y是在螺旋线局部坐标系中的y坐标，单位是米（m）。
* @param t      tangent direction at s [rad]//参数t是在s处的切线方向，单位是弧度（rad）。
*/

extern void odrSpiral( double s, double cDot, double *x, double *y, double *t );
