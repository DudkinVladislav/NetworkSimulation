// Выходной путь к каталогу.
StringPtr outDir = new String("OutputDirectory\\");

// Путь к выходному файлу excel
StringPtr outputChartTypeLine = outDir->StringAppend(new String("outputChartTypeLine.xlsx"));

// Создать новую книгу
intrusive_ptr<IWorkbook> workbook = Factory::CreateIWorkbook();

// Получить первый рабочий лист, созданный по умолчанию
intrusive_ptr<IWorksheet> worksheet = workbook->GetIWorksheets()->GetObjectByIndex(0);

// Добавление образцов значений в ячейки
worksheet->GetICells()->GetObjectByIndex(new String("A1"))->PutValue(50);
worksheet->GetICells()->GetObjectByIndex(new String("A2"))->PutValue(100);
worksheet->GetICells()->GetObjectByIndex(new String("A3"))->PutValue(150);
worksheet->GetICells()->GetObjectByIndex(new String("B1"))->PutValue(4);
worksheet->GetICells()->GetObjectByIndex(new String("B2"))->PutValue(20);
worksheet->GetICells()->GetObjectByIndex(new String("B3"))->PutValue(50);

// Добавление диаграммы на рабочий лист
int chartIndex = worksheet->GetICharts()->Add(Aspose::Cells::Charts::ChartType::ChartType_Line, 5, 0, 20, 8);

// Доступ к экземпляру вновь добавленной диаграммы
intrusive_ptr<Aspose::Cells::Charts::IChart> chart = worksheet->GetICharts()->GetObjectByIndex(chartIndex);

// Добавление SeriesCollection (источника данных диаграммы) на диаграмму в диапазоне от ячейки "A1" до "B3"
chart->GetNISeries()->Add(new String("A1:B3"), true);

// Сохранение файла Excel
workbook->Save(outputChartTypeLine);