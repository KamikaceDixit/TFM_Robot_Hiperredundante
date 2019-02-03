# MACH I. Un manipulador actuado por cables hiperredundante
MACH I es el resultado de un proyecto de fin de máster realizado en la Escuela Superior de Ingenieros Industriales de la UPM. El objetivo de dicho proyecto era construir un robot hiperredundante funcional, que pudiera ser utilizado en un futuro para probar nuevos algoritmos de movimiento en este tipo de robots. El resultado del proyecto es el siguiente.

<div>
<a href="../../tree/master/Renders">
<img src="/Readme_Images/Robot_Completo.png" style="float:center;" width="100%" height="auto"/>
</a>
</div>

Se trata de un robot hiperredundante con 17 grados de libertad. Se compone de 7 rótulas con 2 grados de libertad cada una una, más 3 gdl extra para poder accionar una pinza o cualquier otro elemento en el extremo.
Cada rótula esta controlada con 3 cables equidistantes 120º y el rango de giro de cada una es de 33º con respecto a cualquier eje contenido en el plano paralelo a la base fija de la rótula que pase por el centro de la misma.

La concepción de MACH I se divide en 3 partes:
- Parte mecánica
- Parte electrónica y eléctrica
- Parte de programación
<hr />
En el repositorio, el proyecto esta descompuesto en varias carpetas.

### CAD y Planos
En dicha carpeta se encuentran todos los archivos CAD pertenecientes la proyecto. Estan desarrollados en CATIA V5 R21.
### Control Dinámico
Se recogen los ficheros de Matlab y diversas pruebas realizadas para caracterizar el conjunto "control+motor" para poder ajustar los diferentes parametros del control. En él se encuentran diversos archivos de Matlab y Simulink.
### PCBs
Se encuentran todos los ficheros utilizados para realizar las placas de electrónica, incluyendo esquemáticos, pcbs y librerías.
### Programas
Aquí estan desarrollados los programas integrados en cada placa y la Interfaz Hombre Máquina desarrollada en Matlab.
### Renders
Carpeta que recoge varios Renders realizados a partir del CAD.
<hr />

Adicionalmente, se recoge diverso material gráfico del proyecto en Flickr y YouTube, y puede ser consultado en los siguiente enlaces:
<div align="center">
<a href="https://www.flickr.com/photos/156143787@N03/collections/72157706450447504/">
<img src="/Readme_Images/Flickr_logo.png" style="float:center;" width="30%" height="auto" hspace="30"/>
</a>
<a href="https://www.youtube.com/playlist?list=PLwVt_GYxzsi9YXO1_7UndzVlK2NOCYKW8">
<img src="/Readme_Images/Youtube_Logo.png" style="float:center;" width="30%" height="auto" hspace="30"/>
</a>
</div>

<hr />


<a data-flickr-embed="true" data-footer="true" data-context="true"  href="https://www.flickr.com/photos/156143787@N03/46243925134/" title="Robot_3"><img src="https://farm5.staticflickr.com/4900/46243925134_01e17ef78d_k.jpg" width="100%" height="auto" alt="Robot_3"></a>

