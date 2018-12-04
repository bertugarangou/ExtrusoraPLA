# Extrusora PLA

Extrusora de PLA controlada amb Arduino Mega 2560.

### Prerequisits
En el cas que es vulgui construir:

És necessari el següent material en especial:
* **Arduino Mega 2650 (REV3).** Original o rèplica.
* **Extrusora muntada.** Tremuga, compartiment amb vis-sens-fi, boquilla, sistema de refrigeració i sistema de bobinatge.
* **Motors pas-a-pas NEMA 17 o 23.** Per la bobina i pel vis sens fi.
* **Bobina.** Per guardar el fil obtingut.
* **Font d'alimentació.** Per l'Arduino, ventiladors i motors i resistència, de 230V, 12V i 9V/5.05V.

També és recomanable entendre el llenguatge de programació C i C++ i el funcionament de la plataforma Arduino i Arduino IDE.

#### Peces en 3D

 Algunes peces han sigut dissenyades des de zero i impreses en 3D (PLA), es poden trobar a [Parts3D](/Parts). Per dissenyar-les, s'ha utilitzat el programa [Autodesk Inventor Pro 2018/2019](https://www.autodesk.com/products/inventor/overview) i [Fusion 360](https://www.autodesk.com/products/fusion-360/overview), per tant, els fitxers penjats estan en el format ".ipt" o ".dwg" si es tracta d'un esquema (plànol). A més a més, estan penjades les mateixes peces en el format ".stl" per si no es tenen els anteriors programes. Els fitxers ".stl" (STereoLithography/Standard Tessellation Language) permeten ser carregats per un programa com [Ultimaker Cura](https://ultimaker.com/en/products/ultimaker-cura-software) (codi obert) o [Simplify3D](https://www.simplify3d.com/) (no és de codi obert) entre d'altres per convertir-se en un format llegible per les impresores i especificar certes configuracions per l'impresió posterior.
 
D'altres peces, les que estan en contacte amb zones calentes, estan fetes amb acer o alumini. Tot i així, els suports de la base són fets de fusta aglomerada.

Alerta: els fitxers s'han d'obrir amb la versió 2019 per no tenir errors de visualització. Els fitxers ".dwg" s'han d'obrir amb Innventor millor que amb AutoCAD. Els programes d'Autodesk són de llicència gratuita educativa.

### Instal·lació

Simplement descarrega el programa [Extrusora.ino](/Program/Extrusora/Extrusora.ino), compila'l a la placa i ja estarà llest!
També pots descarregar la [versió comentada](/Program/Extrusora_comentat/Extrusora_comentat.ino) per aprendre com funciona i la [versió de deuració](/Program/Extrusora_depuracio/Extrusora_depuracio.ino) per comprovar totes les funcions i dades amb connexió sèrie (valor de refresc: 9600u).
Però espera, primer necessites l'extrusora!

##  Versions

Es pot visitar l'historial de versions (changelog) [aquí](Changelog.md)

## Referències

* [Github](https://github.com/bertugarangou/ExtrusoraPLA/) - Sistema al núvol
* [Arduino](https://www.arduino.cc/) - Software
* [Arduino LiquidCrystal Library](https://www.arduino.cc/en/Reference/LiquidCrystal) - LLibreria d'Arduino per LCD
* [Arduino Tasker Library](https://github.com/joysfera/arduino-tasker) - Llibreria d'Arduino per l'ús multi-tasca
* [Autodesk](https://www.autodesk.com/) - Software
* [Autodesk Inventor](https://www.autodesk.com/products/inventor/overview) - Software CAD 3D (enfocat a usos professionals)
* [Autodesk Fusion](https://www.autodesk.com/products/fusion-360/overview) - Software CAD 3D (enfocat a l'educació)


## Authors

* **Albert Garangou Culebras** - *Autor principal del projecte*

## Llicència

**Tots els drets reservats, Albert Garangou Culebras.**

Podeu trobar una còpia de la llicència a [LICENSE.md](LICENSE.md) i més detalls.

## Reconeixaments

* *Tutor del Treball de Recerca* - **Jordi Fanals Oriol**

## Contacte

Podeu contactar-me al correu [albertgarangou@gmail.com](mailto:albertgarangou@gmail.com)
