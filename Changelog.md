# Historial de versions

## Com funciona:
---
## Versió [NUMERO] (Data) - *CodeName*
 ## <No Funcional>

 ### Millores

  * Descripció
  * Descripció
      * Contingut

 ### Correccions
  * Descripció

 ### Errors coneguts
  * Descripció

 ### Més informació
  * [Commits](https://github.com/bertugarangou/ExtrusoraPLA/compare/0.1...0.1.1)

---

## Versió 0.2 (7/8/2018) - L298n Update

 ### Millores
  * Projecte Fritzing creat.
  * Codi reescrit per funcionar amb la controladora L298n.
    * Afegida la funció per fer un pas als motors pas-a-pas introduint només variables privades.
    
 ### Més informació
  [Commits](https://github.com/bertugarangou/ExtrusoraPLA/compare/0.1.1...0.2)

---
## Versió 0.3 (8/8/2018) - Actualització TB6600

 ### Millores
  * Projecte Fritzing editat.
    * Afegida controladora TB6600 i eliminada L298n.
    * NTC afegida.
  * Afegida la funció doStep() (de control fàcil).
  * Afegits nous components.
 
 ### Errors coneguts
  * Components nous no provats.
  
 ### Més informació
  * [Commits](https://github.com/bertugarangou/ExtrusoraPLA/compare/0.2...0.3)
  
---
## Versió 1 (13/8/2018) - Here-we-go Update

 ### Millores
  * Controls d'interruptors per l'extrusora.
  * Control d'interruptors per la bobina.
  * Controls de seguretat (checks_...).
  * Ara suporta multi-tasca (multitasking).
  * PI (Proporcional i Integral) per la resistència per escalfar
  * Depuració per "Serial USB"
  * Afegits nous components.
    * Pantalla (LCD) de 16x2 (16 pins)
      * Potenciometre per regular el contrast
  
 ### Més informació
  * [Commits](https://github.com/bertugarangou/ExtrusoraPLA/compare/0.3...1.0)
