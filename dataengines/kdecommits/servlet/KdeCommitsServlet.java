import java.io.*;
import java.sql.*;
import java.lang.reflect.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class KdeCommitsServlet extends HttpServlet
{
    public void init()
    {
        String dbHost = "localhost";
        String dbName = "sandros_kde";
        String username = "sandros_kdeob";
        String password = "oBse_4$s";

        try
        {
            Class.forName("com.mysql.jdbc.Driver");
        }
        catch(ClassNotFoundException msg)
        {
            System.out.println("Error loading driver:" + msg.getMessage());
        }

        try
        {
            String url ="jdbc:mysql://" + dbHost + ":3306/" + dbName;
            conn = DriverManager.getConnection(url, username, password);
            stmt = conn.createStatement();
        }
        catch(SQLException e)
        {
            String err1Msg = e.getMessage();
        }
    }

    public void doGet(HttpServletRequest request, HttpServletResponse response)
    {
        String operation = null;
        Class[] paramTypes = null;
        Object[] paramValues = null;

        try
        {
            out = response.getWriter();

            if (request.getParameterMap().size() == 0)
            {
                out.println("No operation supplied !");
                return;
            }

            response.setContentType("text/plain");
            operation = request.getParameter("op");
            paramTypes = new Class[request.getParameterMap().size()-1];
            paramValues = new Object[paramTypes.length];

            for (int i = 0; i < paramValues.length; ++i)
            {
                paramTypes[i] = String.class;
                paramValues[i] = request.getParameter("p" + i);
            }

            Method method = getClass().getDeclaredMethod(operation, paramTypes);
            method.invoke(this, paramValues);
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

    public void allProjectsInfo() throws SQLException
    {
        String query = "select name, commit_subject, krazy_report, krazy_identifier, icon, add_in_view from projects";
        ResultSet res = stmt.executeQuery(query);
        printResultSet(res);
    }

    public void topActiveProjects(String n) throws SQLException
    {
        String query = "select p.name, count(*) from projects p, commits c where INSTR(c.path, p.commit_subject) > 0 group by p.commit_subject order by count(*) desc";
        if (!n.equals("0"))
            query = query + " limit 0 , " + n;
        ResultSet res = stmt.executeQuery(query);
        printResultSet(res);
    }

    public void topProjectDevelopers(String project, String n) throws SQLException
    {
        String query;
        if (!project.equals(""))
            query = "select d.full_name, d.svn_account, d.first_commit, d.last_commit, count(*) from projects p, commits c, developers d where INSTR(c.path, p.commit_subject) > 0 and d.svn_account = c.svn_account and p.name = '" + project + "' group by d.full_name, p.name order by count(*) desc";
        else
            query = "select d.full_name, d.svn_account, d.first_commit, d.last_commit, count(*) from commits c, developers d where d.svn_account = c.svn_account group by d.full_name order by count(*) desc";
        if (!n.equals("0"))
            query = query + " limit 0 , " + n;
        ResultSet res = stmt.executeQuery(query);
        printResultSet(res);
    }

    private void printResultSet(ResultSet res) throws SQLException
    {
        int count = res.getMetaData().getColumnCount();
        while(res.next())
        {
            for (int i = 1; i < count; ++i)
                out.print(res.getObject(i) + ";");
            out.println(res.getObject(count));
        }
    }

    private PrintWriter out;
    private Connection conn;
    private Statement stmt;
}