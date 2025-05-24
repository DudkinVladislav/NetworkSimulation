// �������� ���� � ��������.
StringPtr outDir = new String("OutputDirectory\\");

// ���� � ��������� ����� excel
StringPtr outputChartTypeLine = outDir->StringAppend(new String("outputChartTypeLine.xlsx"));

// ������� ����� �����
intrusive_ptr<IWorkbook> workbook = Factory::CreateIWorkbook();

// �������� ������ ������� ����, ��������� �� ���������
intrusive_ptr<IWorksheet> worksheet = workbook->GetIWorksheets()->GetObjectByIndex(0);

// ���������� �������� �������� � ������
worksheet->GetICells()->GetObjectByIndex(new String("A1"))->PutValue(50);
worksheet->GetICells()->GetObjectByIndex(new String("A2"))->PutValue(100);
worksheet->GetICells()->GetObjectByIndex(new String("A3"))->PutValue(150);
worksheet->GetICells()->GetObjectByIndex(new String("B1"))->PutValue(4);
worksheet->GetICells()->GetObjectByIndex(new String("B2"))->PutValue(20);
worksheet->GetICells()->GetObjectByIndex(new String("B3"))->PutValue(50);

// ���������� ��������� �� ������� ����
int chartIndex = worksheet->GetICharts()->Add(Aspose::Cells::Charts::ChartType::ChartType_Line, 5, 0, 20, 8);

// ������ � ���������� ����� ����������� ���������
intrusive_ptr<Aspose::Cells::Charts::IChart> chart = worksheet->GetICharts()->GetObjectByIndex(chartIndex);

// ���������� SeriesCollection (��������� ������ ���������) �� ��������� � ��������� �� ������ "A1" �� "B3"
chart->GetNISeries()->Add(new String("A1:B3"), true);

// ���������� ����� Excel
workbook->Save(outputChartTypeLine);