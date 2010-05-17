import java.io.*;
import java.sql.*;
import java.lang.reflect.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class KdeCommitsServlet extends HttpServlet
{
    public void init()
    {
    }

    public void doGet(HttpServletRequest request, HttpServletResponse response)
    {
        String operation = null;
        Class[] paramTypes = null;
        Object[] paramValues = null;

        PrintWriter out = null;
        try
        {
            out = response.getWriter();
        }
        catch (IOException e)
        {
            System.out.println("Error acquiring writer:" + e.getMessage());
        }

        try
        {
            if (request.getParameterMap().size() == 0)
            {
                response.setContentType("text/html");
                out.println("No operation supplied !");
                return;
            }

            String dbHost   = "";
            String dbName   = "";
            String username = "";
            String password = "";

            Class.forName("com.mysql.jdbc.Driver");

            String url ="jdbc:mysql://" + dbHost + ":3306/" + dbName;

            Connection conn = DriverManager.getConnection(url, username, password);

            response.setContentType("text/plain");
            operation = request.getParameter("op");
            paramTypes = new Class[request.getParameterMap().size()+1];
            paramValues = new Object[paramTypes.length];

            paramTypes[0] = PrintWriter.class;
            paramValues[0] = out;
            paramTypes[1] = Connection.class;
            paramValues[1] = conn;

            for (int i = 0; i < paramValues.length - 2; ++i)
            {
                paramTypes[i+2] = String.class;
                paramValues[i+2] = request.getParameter("p" + i);
            }

            Method method = getClass().getDeclaredMethod(operation, paramTypes);
            method.invoke(this, paramValues);
            conn.close();
        }
        catch(ClassNotFoundException e)
        {
            System.out.println("Error loading driver:" + e.getMessage());
        }
        catch(SQLException e)
        {
            String err1Msg = e.getMessage();
        }
        catch (NoSuchMethodException e)
        {
            response.setContentType("text/html");
            out.println("Unsupported operation " + operation + " with " + paramTypes.length + " String parameters !");
        }
        catch (Exception e)
        {
            response.setContentType("text/html");
            out.println("Exception: " + e.getClass().getName() + " " + e.getMessage());
        }
    }

    public void allProjectsInfo(PrintWriter out, Connection conn) throws Exception
    {
        try
        {
            String query = "select name, commit_subject, krazy_report, krazy_identifier, icon, add_in_view from projects";
            Statement stmt = conn.createStatement();
            ResultSet res = stmt.executeQuery(query);
            printResultSet(out, res);
        }
        catch(Exception e)
        {
            out.println("Exception: " + e.getClass().getName() + " " + e.getMessage());
            throw e;
        }
    }

    public void topActiveProjects(PrintWriter out, Connection conn, String n, String fromDate, String toDate) throws Exception
    {
        try
        {
            String query = "select p.name, count(*) from projects p, commits c where INSTR(c.path, p.commit_subject) > 0";

            if (!fromDate.equals(""))
                query = query + " and date(c.date_time) >= '" + fromDate + "'";

            if (!toDate.equals(""))
                query = query + " and date(c.date_time) <= '" + toDate + "'";

            query = query + " group by p.commit_subject order by count(*) desc";

            if (!n.equals("0"))
                query = query + " limit 0 , " + n;

            Statement stmt = conn.createStatement();
            ResultSet res = stmt.executeQuery(query);
            printResultSet(out, res);
        }
        catch(Exception e)
        {
            out.println("Exception: " + e.getClass().getName() + " " + e.getMessage());
            throw e;
        }
    }

    public void topProjectDevelopers(PrintWriter out, Connection conn, String project, String n, String fromDate, String toDate) throws Exception
    {
        try
        {
            String query;
            if (!project.equals(""))
                query = "select d.full_name, d.svn_account, d.first_commit, d.last_commit, count(*) from projects p, commits c, developers d where INSTR(c.path, p.commit_subject) > 0 and d.svn_account = c.svn_account and p.name = '" + project + "'";
            else
                query = "select d.full_name, d.svn_account, d.first_commit, d.last_commit, count(*) from commits c, developers d where d.svn_account = c.svn_account";

            if (!fromDate.equals(""))
                query = query + " and date(c.date_time) >= '" + fromDate + "'";

            if (!toDate.equals(""))
                query = query + " and date(c.date_time) <= '" + toDate + "'";

            if (!project.equals(""))
                query = query + " group by d.full_name, p.name order by count(*) desc";
            else
                query = query + " group by d.full_name order by count(*) desc";

            if (!n.equals("0"))
                query = query + " limit 0 , " + n;

            Statement stmt = conn.createStatement();
            ResultSet res = stmt.executeQuery(query);
            printResultSet(out, res);
        }
        catch(Exception e)
        {
            out.println("Exception: " + e.getClass().getName() + " " + e.getMessage());
            throw e;
        }
    }

    public void commitHistory(PrintWriter out, Connection conn, String project, String n, String fromDate, String toDate) throws Exception
    {
        try
        {
            String query;
            if (!project.equals(""))
                query = "select date(c.date_time) date, count(*) from projects p, commits c where INSTR(c.path, p.commit_subject) > 0 and p.name = '" + project + "'";
            else
                query = "select date(c.date_time) date, count(*) from commits c";
            
            if (!fromDate.equals(""))
                query = query + " and date(c.date_time) >= '" + fromDate + "'";

            if (!toDate.equals(""))
                query = query + " and date(c.date_time) <= '" + toDate + "'";

            if (!project.equals(""))
            query = query + " group by date order by date asc";

            if (!n.equals("0"))
                query = query + " limit 0 , " + n;

            Statement stmt = conn.createStatement();
            ResultSet res = stmt.executeQuery(query);
            printResultSet(out, res);
        }
        catch(Exception e)
        {
            out.println("Exception: " + e.getClass().getName() + " " + e.getMessage());
            throw e;
        }
    }

    private void printResultSet(PrintWriter out, ResultSet res) throws Exception
    {
        try
        {
        int count = res.getMetaData().getColumnCount();
        while(res.next())
        {
            for (int i = 1; i < count; ++i)
                out.print(res.getObject(i) + ";");
            out.println(res.getObject(count));
        }
        }
        catch(Exception e)
        {
            out.println("printResultSet Exception: " + e.getClass().getName() + " " + e.getMessage());
            throw e;
        }
    }
}