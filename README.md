# Radio Internetowe
Michał Jaruzel inf145339 i4.2

Opis projektu:

    Użytkownik łączy się do serwera, podając Nick.

    Serwer sprawdza dostępność nicku i jeżeli jest zajęty, prosi o wprowadzenie innego.

    Po dołączeniu do serwera użytkownik otrzymuje listę nazw pokoi wraz z liczbą osób się w nich znajdującą.

    Użytkownik może dołączyć do istniejącego pokoju lub stworzyć nowy wybierając jego nazwę (musi być unikalna).

    Po dołączeniu do pokoju użytkownik otrzymuje:
    - nazwę aktualnie granego utworu,
    - nicki innych użytkowników w pokoju,
    - listę nazw utworów w pokoju uporządkowana według liczby głosów
    Wszystkie te informacje są na bieżąco aktualizowane co jakiś ustalony krótki czas.

    Użytkownik może przesłać kolejny utwór, podając przy tym jego nazwę, która musi być unikalna na liście utworów w pokoju.

    Każdy użytkownik może głosować w danej chwili tylko na jeden utwór, ale zawsze może zmienić swoją decyzję.

    Po zakończeniu utworu z listy wybierany jest kolejny posiadający na daną chwilę najwięcej głosów, jeżeli jest takich kilka, utwór jest losowany, w skrajnym przypadku, jeżeli nikt na nic nie zagłosował, utwory odtwarzane są losowo z całej listy utworów. Po rozpoczęciu odtwarzania utworu, głosy oddane na  niego są usuwane.

    Połączenie z użytkownikami jest monitorowane i jeżeli przez określony na serwerze czas użytkownik nie odpowiada, zostaje on usunięty z pokoju i listy użytkowników na serwerze (jego nick jest wolny).
    Jeżeli w pokoju nie ma użytkowników przez określony na serwerze czas (np. 5 minut), pokój jest usuwany. 

## TODO
- [x] server udp - sending timer
- [ ] sending song file info
- [ ] vote change
- [ ] repair send songs


![eventsGraph.png](https://github.com/D37ERTER/Radio-Internetowe/blob/main/eventsGraph.png?raw=true)
